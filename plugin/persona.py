"""Per-profile character appearance and behaviour: what a device draws and how he acts.

The record is the ``persona`` wire message the firmware takes (``App::h_persona`` in
``firmware/core/src/app.cpp``): which costume Potsy wears, which backdrop the hero band paints,
and the knobs that used to be compile-time constants in ``firmware/core/src/potsy/settings.h`` —
his idle chatter, how fast he animates, and whether he stands on the clock face.

Lives in the plugin's data directory (``<hermes home>/plugin-data/herpots/``), like
``devices.json``. The file is one object keyed by Hermes profile name; each record holds every
field, so a reader never has to guess a default. A profile without a record, a missing or
unreadable file reads as :data:`DEFAULTS`.

The dashboard's routes (``dashboard/plugin_api.py``) and the gateway (``adapter.py``) edit the
same file from different processes, so like ``store.DeviceStore`` every operation re-reads the
file first, and a write goes through a temporary file, so neither side overwrites the other's
change with a stale copy.
"""

from __future__ import annotations

import json
import os
import tempfile
from pathlib import Path
from typing import Any

FILENAME = "persona.json"

# What the firmware can draw and do. The first two are table lengths in the firmware
# (``OUTFITS``/``OutfitId::COUNT`` in potsy.cpp, ``Settings::Background``); the rest are the
# ranges ``App::h_persona`` clamps to. A value outside them never reaches a device.
OUTFITS = 13        # costume values: 0 (none) .. 12, the wardrobe's current length
BACKGROUNDS = 5     # backdrops: 0 (digital rain) .. 4 (black)
BANTER = ("ТИХИЙ", "ОБЫЧНЫЙ", "БОЛТЛИВЫЙ")  # 0, 1, 2 — idle chatter off, normal, more often
TEMPO_MIN, TEMPO_MAX = 50, 200              # Potsy's animation speed, percent of the designed one
MINI = (False, True)                        # whether he also stands on the clock face
MAX_PROFILE = 64  # a Hermes profile name's length, bounded so a bad request cannot bloat the file

DEFAULTS: dict[str, Any] = {"outfit": 0, "background": 0, "banter": 1, "tempo": 100, "mini": True}


def _clamped(value: Any, limit: int) -> int:
    """An index inside its range; anything else reads as the default."""
    return value if isinstance(value, int) and not isinstance(value, bool) and 0 <= value < limit else 0


def _tempo(value: Any) -> int:
    if isinstance(value, int) and not isinstance(value, bool) and TEMPO_MIN <= value <= TEMPO_MAX:
        return value
    return DEFAULTS["tempo"]


def _record(raw: Any) -> dict[str, Any]:
    rec = raw if isinstance(raw, dict) else {}
    banter = rec.get("banter")
    return {
        "outfit": _clamped(rec.get("outfit"), OUTFITS),
        "background": _clamped(rec.get("background"), BACKGROUNDS),
        "banter": banter if isinstance(banter, int) and not isinstance(banter, bool)
                  and 0 <= banter < len(BANTER) else DEFAULTS["banter"],
        "tempo": _tempo(rec.get("tempo")),
        "mini": rec["mini"] if isinstance(rec.get("mini"), bool) else DEFAULTS["mini"],
    }


def _load(directory: Path | str) -> dict[str, Any]:
    try:
        raw = json.loads((Path(directory) / FILENAME).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return {}
    return raw if isinstance(raw, dict) else {}


def wire(record: dict[str, Any]) -> dict[str, Any]:
    """The fields the ``persona`` message carries: what the device draws and how he behaves.

    The names are the firmware's (``App::h_persona``), so a reader of the wire log can match them
    to the handler; ``banter`` is the chatter level, ``tempo`` the animation speed in percent.
    """
    return {"outfit": record["outfit"], "background": record["background"],
            "banter": record["banter"], "tempo": record["tempo"], "mini": record["mini"]}


def settings(directory: Path | str, profile: str) -> dict[str, Any]:
    """``profile``'s record; the defaults when it has none."""
    record = _load(directory).get(profile)
    return _record(record) if isinstance(record, dict) else dict(DEFAULTS)


def all(directory: Path | str) -> dict[str, dict[str, Any]]:
    """Every profile's record, as the file holds it (unknown values read as the defaults)."""
    return {str(k): _record(v) for k, v in _load(directory).items() if isinstance(v, dict)}


def save(directory: Path | str, profile: str, **changes: Any) -> dict[str, Any]:
    """Set the named fields of ``profile``'s record (the rest keep what they had), returning it.

    Raises ValueError with a sentence on a profile name or a value the device would not take.
    """
    if not isinstance(profile, str) or not profile.strip() or len(profile) > MAX_PROFILE:
        raise ValueError("name the profile the look belongs to")
    profile = profile.strip()
    unknown = set(changes) - set(DEFAULTS)
    if unknown:
        raise ValueError(f"not a persona field: {', '.join(sorted(unknown))}")
    changes = {k: v for k, v in changes.items() if v is not None}
    known = {k: v for k, v in changes.items()}
    if "outfit" in known and (not isinstance(known["outfit"], int) or isinstance(known["outfit"], bool)
                              or not 0 <= known["outfit"] < OUTFITS):
        raise ValueError(f"outfit is 0 to {OUTFITS - 1}")
    if "background" in known and (not isinstance(known["background"], int) or isinstance(known["background"], bool)
                                  or not 0 <= known["background"] < BACKGROUNDS):
        raise ValueError(f"background is 0 to {BACKGROUNDS - 1}")
    if "banter" in known and (not isinstance(known["banter"], int) or isinstance(known["banter"], bool)
                              or not 0 <= known["banter"] < len(BANTER)):
        raise ValueError(f"banter is 0 to {len(BANTER) - 1}")
    if "tempo" in known and (not isinstance(known["tempo"], int) or isinstance(known["tempo"], bool)
                             or not TEMPO_MIN <= known["tempo"] <= TEMPO_MAX):
        raise ValueError(f"tempo is {TEMPO_MIN} to {TEMPO_MAX}")
    if "mini" in known and not isinstance(known["mini"], bool):
        raise ValueError("mini is true or false")

    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    path = directory / FILENAME
    data = _load(directory)
    record = _record(data.get(profile))
    record.update(changes)
    data[profile] = record
    fd, tmp = tempfile.mkstemp(dir=directory, prefix=".persona.", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, sort_keys=True)
        os.replace(tmp, path)
    except BaseException:
        Path(tmp).unlink(missing_ok=True)
        raise
    return record
