# Draft PR text for NousResearch/hermes-agent (submit after owner review)

PR target: add `plugin-catalog/herpots.yaml` to NousResearch/hermes-agent.

## plugin-catalog/herpots.yaml (proposed content)

```yaml
name: herpots
description: >-
  HerPots companions: small ESP32-S3 devices (screen, microphone, speaker) with
  the Potsy character and a Russian-first UI. Devices hold a WebSocket to the
  gateway; voice is transcribed by Hermes STT, replies are shown on the device
  and spoken with Hermes TTS. A desktop page includes a virtual companion you
  can talk to from the app. Firmware staging for the devices is included
  (hermes gadget update); it never modifies the plugin itself. Disclosures:
  platform-kind adapter imports gateway internals (see rule 9 discussion in the
  PR body); device UI ships Russian-first by owner choice, English available in
  the device menu; GPL-3.0-or-later because the Potsy character comes from the
  author's PotsCharm project.
repo: https://github.com/pusi-os/herpots
subdir: plugin
sha: <40-hex commit pinned at review time>
tier: community
category: platform
maintainer: pusi-os (teletonn)
capabilities:
  tools: [gadget_devices, gadget_display, gadget_ask, gadget_speak, gadget_action,
          gadget_play, gadget_widget, gadget_interpret]
  hooks: [on_session_start, pre_llm_call, on_session_end, pre_approval_request,
          post_approval_response, on_session_finalize, on_session_reset,
          pre_transcription]
  env: [GADGET_ACCESS_TOKEN, GADGET_ALLOWED_USERS, GADGET_ALLOW_ALL_USERS]
requires_hermes: ">=0.21.6"
title: HerPots
version: "0.7.0"
docs_url: https://github.com/pusi-os/herpots#readme
```

## PR body draft

**What it does.** HerPots connects small hardware companions (ESP32-S3 boards with a screen,
microphone and speaker) to Hermes: hold a button, ask, hear the answer. The plugin is a
`kind: platform` fork of the upstream `gadget` plugin by Adolanium (MIT, already cataloged),
extended with: the Potsy character (ported from the author's GPL-3.0 PotsCharm firmware),
a Russian-first device UI, and two board definitions (xiaozhi-cube-154, zhengchen-154).

**Surfaces used.** `ctx.register_platform`, `ctx.register_cli_command`, dashboard backend API,
desktop plugin SDK (a Gadgets page with a virtual companion), hooks (session state,
pre_transcription language), 8 tools (`gadget_*`, names kept from upstream so devices flashed
with the parent firmware keep working).

**License.** GPL-3.0-or-later: the Potsy character is ported from PotsCharm (GPL-3.0), which the
author owns; the MIT-licensed upstream parts (hermes-gadget-sdk) remain MIT on their own, the
attribution is in NOTICE.md at the pinned commit.

**Disclosures (rule 13).**

- Platform-kind adapter imports gateway internals (STT/TTS tools, pairing store, approval
  timeout, profile listing, URL safety) so device sessions behave exactly like local ones.
  The full seam inventory is documented in `docs/herpots-catalog-plan.md` and at the import
  site in `plugin/adapter.py`; graceful fallbacks are added where the behavior can degrade.
  We will open a tracking issue for the seams we would like promoted to public APIs.
- The device UI is Russian-first by owner choice; English ships in the device menu.
- `hermes gadget update` stages ESP32 firmware images onto the devices themselves. It never
  reads or writes the plugin's own files; releases are fetched from GitHub by pinned tag, so
  this is a checksum-pinned download, not a self-updater.
- Screenshots: (to attach) desktop page with the virtual companion; device dashboard.

**Validation.** `hermes plugins validate plugin --install-deps` passes at the pinned commit
(manifest, capabilities, security scan safe, no core override, desktop surface inside the SDK).

## Draft issue text for NousResearch/hermes-agent (P5, file only — not posted)

Title: Public seams for kind:platform plugins (STT/TTS/approvals/pairing/profiles)

Body:

`kind: platform` plugins necessarily run in the gateway process, but the pieces a platform
adapter needs are spread across private modules. HerPots (pusi-os/herpots, a fork of the
upstream gadget plugin) currently imports:

- `gateway.platforms.base` — the adapter base classes (`BasePlatformAdapter`, `SendResult`,
  `AudioFormat`, `StreamingTTSHandle`, `ExecApprovalPrompt`, audio/image cache helpers)
- `gateway.platforms.event` — `MessageEvent`, `MessageType`
- `gateway.platforms._shared` — `extra_or_secret`, `get_scoped_secret`
- `gateway.pairing` — `PairingStore`
- `gateway.platforms.base_exec_approval` — `approval_timeout_seconds`
- `gateway.session_context` — `get_session_env`
- `tools.transcription_tools` / `tools.tts_tool` — device voice pipelines
- `tools.url_safety` — SSRF-safe fetches for links shown on the device
- `tools.approval` — `restrict_gateway_approval_card_to_deny`
- `agent.i18n` — reply language for the device
- `agent.secret_scope`, `hermes_cli.profiles`, `hermes_cli.config`, `hermes_cli.setup`,
  `hermes_constants` — profile homes/roster, config write, setup prompts, HERMES_HOME

Would you consider promoting a stable surface for these (or documenting them as allowed
seams for platform plugins in `hermes plugins validate`)? Happy to contribute the PR.
