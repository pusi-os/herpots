"""Per-profile character appearance: the outfit and background a device draws, in ``persona.json``.

Lives in the plugin's data directory (``<hermes home>/plugin-data/herpots/``), like
``devices.json``. The file is one object keyed by Hermes profile name; each record is
``{"outfit": int, "background": int}`` — the values of the wire message ``persona`` (the
device side is ``firmware/core/include/hg/protocol.hpp``). A profile without a record,
a missing or unreadable file reads as 0, 0: no costume, the default background.

The dashboard's routes (``dashboard/plugin_api.py``) and the gateway (``adapter.py``)
edit the same file from different processes, so like ``store.DeviceStore`` every
operation re-reads the file first, and a write goes through a temporary file, so neither
side overwrites the other's change with a stale copy.
"""

from __future__ import annotations

import json
import os
import tempfile
from pathlib import Path
from typing import Any

FILENAME = "persona.json"
OUTFITS = 20      # outfit values the firmware draws: 0 (none) .. 19
BACKGROUNDS = 12  # background values it draws: 0 (digital rain) .. 11
MAX_PROFILE = 64  # a Hermes profile name's length, bounded so a bad request cannot bloat the file


def _clamped(value: Any, limit: int) -> int:
    """An outfit or background value inside its range; anything else reads as 0."""
    return value if isinstance(value, int) and not isinstance(value, bool) and 0 <= value < limit else 0


def _record(raw: Any) -> dict[str, int]:
    rec = raw if isinstance(raw, dict) else {}
    return {"outfit": _clamped(rec.get("outfit"), OUTFITS),
            "background": _clamped(rec.get("background"), BACKGROUNDS)}


def settings(directory: Path | str, profile: str) -> dict[str, int]:
    """``profile``'s outfit and background, as ``{"outfit", "background"}``; 0, 0 without one."""
    try:
        raw = json.loads((Path(directory) / FILENAME).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        raw = None
    if not isinstance(raw, dict) or not isinstance(raw.get(profile), dict):
        return {"outfit": 0, "background": 0}
    return _record(raw[profile])


def all(directory: Path | str) -> dict[str, dict[str, int]]:
    """Every profile's record, as the file holds it (unknown values read as 0, 0)."""
    try:
        raw = json.loads((Path(directory) / FILENAME).read_text(encoding="utf-8"))
    except (OSError, ValueError):
        raw = None
    if not isinstance(raw, dict):
        return {}
    return {str(k): _record(v) for k, v in raw.items() if isinstance(v, dict)}


def save(directory: Path | str, profile: str, outfit: int | None, background: int | None) -> dict[str, int]:
    """Set ``profile``'s outfit and/or background (None keeps what it had) and return the record.

    Raises ValueError with a sentence on a profile name or value the device would not take.
    """
    if not isinstance(profile, str) or not profile.strip() or len(profile) > MAX_PROFILE:
        raise ValueError("name the profile the look belongs to")
    profile = profile.strip()
    if outfit is not None and (not isinstance(outfit, int) or isinstance(outfit, bool) or not 0 <= outfit < OUTFITS):
        raise ValueError(f"outfit is 0 to {OUTFITS - 1}")
    if background is not None and (not isinstance(background, int) or isinstance(background, bool)
                                   or not 0 <= background < BACKGROUNDS):
        raise ValueError(f"background is 0 to {BACKGROUNDS - 1}")
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    path = directory / FILENAME
    try:
        raw = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        raw = None
    data: dict[str, Any] = raw if isinstance(raw, dict) else {}
    current = _record(data.get(profile))
    record = {"outfit": current["outfit"] if outfit is None else outfit,
              "background": current["background"] if background is None else background}
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
