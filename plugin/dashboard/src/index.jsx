// The HerPots tab of the Hermes web dashboard: the devices this Hermes knows, and a diagnostics
// report from each connected one.
//
// Source of dist/index.js. Build it with `npm run build:dashboard-tab` in site/ (esbuild, React
// from the dashboard's plugin SDK, IIFE output); site's `npm test` fails when dist/index.js is
// older than this file. React and the UI components come from window.__HERMES_PLUGIN_SDK__.

const SDK = window.__HERMES_PLUGIN_SDK__;
const React = SDK.React;
const { useCallback, useEffect, useState } = SDK.hooks;
const { Badge, Button, Card, CardContent, CardHeader, CardTitle } = SDK.components;
const { timeAgo } = SDK.utils;

const API = "/api/plugins/herpots";

// The dashboard adds the selected profile to its own profile-scoped requests, but not to plugin
// routes (/api/plugins/ is not among them). The route resolves the plugin's data directory from
// `?profile=`, so pass along the one the dashboard page was opened with, when there is one.
let latestRequest = 0;  // the number of the last /devices request the tab made
let activeRequest = null;  // the AbortController of the request in flight, cancelled on leaving the tab

function apiUrl(path) {
  const profile = new URLSearchParams(window.location.search).get("profile");
  return profile ? `${API}${path}?profile=${encodeURIComponent(profile)}` : `${API}${path}`;
}

function devicesUrl() {
  return apiUrl("/devices");
}
const REFRESH_MS = 10000;
// The SDK's fetch has no timeout: a request that never answers would stop the polling for good.
const REQUEST_TIMEOUT_MS = 15000;

// What a device draws and how the character behaves over Hermes (the persona message), in the
// order the firmware numbers them. The look is saved per Hermes profile, not per device; the
// devices answering a profile get it at their next welcome and on every change.
const OUTFITS = [
  "НЕТ", "ТАНОУКИ", "ЕДИНОРОГ", "ФОЛЬГА", "ТЕНЬ", "САНТЕХНИК", "ВЫСОКИЙ БРАТ",
  "СИНИЙ ВИХРЬ", "КАПИТАН", "ХРОМОВОЕ КРЫЛО", "ГЛАЗ ПУСТОТЫ", "ЗИМНЯЯ ПАРКА", "TH3 0N3",
];
const BACKGROUNDS = [
  "ЦИФРОВОЙ ДОЖДЬ", "ЗВЁЗДНОЕ НЕБО", "ТОСТЕРЫ", "СВЕТЛЯЧКИ", "ЧЁРНЫЙ",
];
const BANTER = ["ТИХИЙ", "ОБЫЧНЫЙ", "БОЛТЛИВЫЙ"];
const TEMPOS = [60, 80, 100, 120, 150, 200];
const MINI = ["СКРЫТ", "НА ЧАСАХ"];

function errorText(err) {
  return String((err && err.message) || err).replace(/^Error:\s*/, "");
}

function when(ts) {
  if (typeof ts !== "number") return "-";
  return timeAgo(ts);
}

function fullDate(ts) {
  return typeof ts === "number" ? new Date(ts * 1000).toLocaleString() : "";
}

// Newer dashboards style a Badge by `tone`, older ones by `variant`; each ignores the other.
function Status({ device }) {
  const pending = device.pending ? <Badge tone="warning" variant="outline">Waiting to pair</Badge> : null;
  // A device that is connected but not yet approved is both: online, and waiting to pair.
  if (device.online) {
    return <span className="inline-flex gap-1"><Badge tone="success" variant="default">Online</Badge>{pending}</span>;
  }
  return pending || <Badge tone="outline" variant="secondary">Offline</Badge>;
}

function Battery({ battery }) {
  if (!battery) return <span className="text-muted-foreground">-</span>;
  const parts = [];
  if (typeof battery.battery_percent === "number") parts.push(`${Math.round(battery.battery_percent)}%`);
  else if (typeof battery.battery_mv === "number") parts.push(`${(battery.battery_mv / 1000).toFixed(2)} V`);
  if (battery.charging) parts.push("charging");
  else if (battery.external_power) parts.push("on USB");
  if (battery.battery_present === false && !parts.length) parts.push("no battery");
  return <span>{parts.join(", ") || "-"}</span>;
}

function GatewayNote({ gateway, count }) {
  if (gateway.running || !count) return null;
  const since = typeof gateway.updated_at === "number" ? ` Its last report was ${when(gateway.updated_at)}.` : "";
  return (
    <p className="text-sm text-muted-foreground">
      The gateway is not running, or has not reported yet, so online state and battery are unknown.{since}
      {" "}Start it with <code>hermes gateway run</code>.
    </p>
  );
}

// Saves a connected device's diagnostics report, with the gateway's recent log, as a text file:
// what `hermes herpots diag` saves, for a bug report.
function SaveReport({ device }) {
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");
  const save = useCallback(() => {
    setBusy(true);
    setError("");
    SDK.fetchJSON(apiUrl(`/devices/${encodeURIComponent(device.id)}/diag`), { method: "POST" })
      .then((r) => {
        if (!r || !r.ok) throw new Error((r && r.error) || "The report could not be taken");
        const url = URL.createObjectURL(new Blob([r.text || ""], { type: "text/plain" }));
        const link = document.createElement("a");
        link.href = url;
        link.download = r.file || "herpots-diag.txt";
        document.body.appendChild(link);
        link.click();
        link.remove();
        setTimeout(() => URL.revokeObjectURL(url), 1000);
      })
      .catch((e) => setError(errorText(e)))
      .finally(() => setBusy(false));
  }, [device.id]);
  if (!device.online) return null;
  return (
    <div className="flex flex-col gap-1">
      <Button variant="outline" size="sm" onClick={save} disabled={busy}>
        {busy ? "Asking the device..." : "Save diagnostics report"}
      </Button>
      {error && <span className="text-xs text-destructive">{error}</span>}
    </div>
  );
}

// One profile's look and behaviour, saved for the Hermes profile answering a device: the costume
// and backdrop its screen draws, how much he chatters, how fast he animates, and whether he also
// stands on the clock face. The controls show the profile's stored values and save the whole
// record, so changing one keeps the rest.
function Persona({ device, persona, onSave }) {
  const record = persona[device.profile] || {};
  const outfit = typeof record.outfit === "number" ? record.outfit : 0;
  const background = typeof record.background === "number" ? record.background : 0;
  const banter = typeof record.banter === "number" ? record.banter : 1;
  const tempo = typeof record.tempo === "number" ? record.tempo : 100;
  const mini = typeof record.mini === "boolean" ? record.mini : true;
  const select = "w-full px-1 py-0.5 text-xs rounded border border-input bg-background";
  const save = (changes) => onSave(device.profile, { outfit, background, banter, tempo, mini, ...changes });
  const label = "text-[10px] uppercase text-muted-foreground";
  return (
    <div className="flex flex-col gap-1" aria-label={`Персонаж (${device.profile})`}>
      <div className={label}>Костюм</div>
      <select className={select} value={outfit} aria-label="Костюм"
              onChange={(e) => save({ outfit: Number(e.target.value) })}>
        {OUTFITS.map((name, i) => <option key={name} value={i}>{name}</option>)}
      </select>
      <div className={label}>Фон</div>
      <select className={select} value={background} aria-label="Фон"
              onChange={(e) => save({ background: Number(e.target.value) })}>
        {BACKGROUNDS.map((name, i) => <option key={name} value={i}>{name}</option>)}
      </select>
      <div className={label}>Болтовня</div>
      <select className={select} value={banter} aria-label="Болтовня"
              onChange={(e) => save({ banter: Number(e.target.value) })}>
        {BANTER.map((name, i) => <option key={name} value={i}>{name}</option>)}
      </select>
      <div className={label}>Темп</div>
      <select className={select} value={tempo} aria-label="Темп"
              onChange={(e) => save({ tempo: Number(e.target.value) })}>
        {TEMPOS.map((t) => <option key={t} value={t}>{t}%</option>)}
      </select>
      <div className={label}>На часах</div>
      <select className={select} value={mini ? 1 : 0} aria-label="Потсик на часах"
              onChange={(e) => save({ mini: Number(e.target.value) === 1 })}>
        {MINI.map((name, i) => <option key={name} value={i}>{name}</option>)}
      </select>
    </div>
  );
}

function DeviceTable({ devices, persona, onSavePersona }) {
  const cell = "px-3 py-2 text-left align-top";
  return (
    <div className="overflow-x-auto">
      <table className="w-full text-sm">
        <thead className="text-xs uppercase text-muted-foreground">
          <tr>
            <th className={cell}>Device</th>
            <th className={cell}>Status</th>
            <th className={cell}>Battery</th>
            <th className={cell}>Board</th>
            <th className={cell}>Firmware</th>
            <th className={cell}>Last seen</th>
            <th className={cell}>Персонаж</th>
            <th className={cell}><span className="sr-only">Report</span></th>
          </tr>
        </thead>
        <tbody>
          {devices.map((d) => (
            <tr key={d.id} className="border-t border-border">
              <td className={cell}>
                <div className="font-medium">{d.name || d.id}</div>
                <div className="font-mono text-xs text-muted-foreground">{d.id}</div>
              </td>
              <td className={cell}><Status device={d} /></td>
              <td className={cell}><Battery battery={d.online ? d.battery : null} /></td>
              <td className={cell}>{d.board || "-"}</td>
              <td className={cell + " font-mono"}>{d.firmware || "-"}</td>
              <td className={cell} title={fullDate(d.online ? d.connected_at : d.last_seen)}>
                {d.online ? "now" : when(d.last_seen)}
              </td>
              <td className={cell}><Persona device={d} persona={persona} onSave={onSavePersona} /></td>
              <td className={cell}><SaveReport device={d} /></td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

function HerPotsPage() {
  const [data, setData] = useState(null);
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);
  const [persona, setPersona] = useState({});
  const [personaError, setPersonaError] = useState("");

  const load = useCallback(() => {
    // Only the latest request's answer is shown, should a Refresh ever overlap a poll.
    const request = ++latestRequest;
    setLoading(true);
    const abort = new AbortController();
    activeRequest = abort;
    const timeout = setTimeout(() => abort.abort(), REQUEST_TIMEOUT_MS);
    return SDK.fetchJSON(devicesUrl(), { signal: abort.signal })
      .then((d) => { if (request === latestRequest) { setData(d); setError(""); } })
      .catch((e) => {
        // The SDK wraps an abort before the response headers in its own error, so ask the signal.
        if (request === latestRequest)
          setError(abort.signal.aborted ? "The Hermes dashboard did not answer in time" : errorText(e));
      })
      .finally(() => {
        clearTimeout(timeout);
        if (request === latestRequest) setLoading(false);
      });
  }, []);

  const loadPersona = useCallback(() => {
    SDK.fetchJSON(apiUrl("/persona"))
      .then((r) => { if (r && r.ok) setPersona(r.persona || {}); })
      .catch(() => {}); // the looks read as none without them; the next save writes them again
  }, []);

  useEffect(() => {
    loadPersona();
  }, [loadPersona]);

  const savePersona = useCallback((profile, changes) => {
    // Shown at once; a push the gateway didn't take is reverted from the store and reported.
    const before = persona;
    setPersona((p) => ({ ...p, [profile]: { ...(p[profile] || {}), ...changes } }));
    setPersonaError("");
    SDK.fetchJSON(apiUrl("/persona"), {
      method: "PUT",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ profile, ...changes }),
    })
      .then((r) => {
        if (!r || !r.ok) throw new Error((r && r.error) || "The look was not saved");
      })
      .catch((e) => {
        setPersona(before);
        setPersonaError(`${profile}: ${errorText(e)}`);
      });
  }, [persona]);

  useEffect(() => {
    // The next poll starts REFRESH_MS after this one settles, not on a fixed clock: a response
    // slower than REFRESH_MS is then still shown, rather than replaced by the next request's.
    let timer = null;
    let stopped = false;
    const poll = () => load().finally(() => { if (!stopped) timer = setTimeout(poll, REFRESH_MS); });
    poll();
    return () => {
      // Leaving the tab: no next poll, and the request in flight is cancelled and its answer ignored.
      stopped = true;
      clearTimeout(timer);
      ++latestRequest;
      if (activeRequest) activeRequest.abort();
    };
  }, [load]);

  const devices = (data && data.devices) || [];
  const online = devices.filter((d) => d.online).length;

  return (
    <div className="flex flex-col gap-4">
      <Card>
        <CardHeader>
          <div className="flex items-center justify-between gap-2">
            <CardTitle>HerPots</CardTitle>
            <Button variant="outline" size="sm" onClick={load} disabled={loading}>Refresh</Button>
          </div>
          {data && (
            <p className="text-sm text-muted-foreground">
              {devices.length} known, {online} online.
            </p>
          )}
        </CardHeader>
        <CardContent>
          <div className="flex flex-col gap-3">
            {error && <p className="text-sm text-destructive">Could not load the devices: {error}</p>}
            {data && <GatewayNote gateway={data.gateway} count={devices.length} />}
            {personaError && <p className="text-sm text-destructive">{personaError}</p>}
            {data && !devices.length && (
              <p className="text-sm text-muted-foreground">
                No companions have connected yet. Set up a board from the installer, then approve it
                with <code>hermes herpots pair</code>.
              </p>
            )}
            {devices.length > 0 && (
              <DeviceTable devices={devices} persona={persona} onSavePersona={savePersona} />
            )}
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

window.__HERMES_PLUGINS__.register("herpots", HerPotsPage);
