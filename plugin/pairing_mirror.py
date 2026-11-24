"""One pairing is every bot's: an approval the default profile gave is mirrored into each served
profile's pairing store, so a device runs under any bot without a second pairing round.

This is what made session switching look broken: the switch itself worked, but the first request
under the new bot met that profile's own pairing gate, whose answer came back as a chat message
telling the user to run the pairing command by hand — and no approve request ever reached the
device's panel. The default profile's verdict is the owner's; the other bots on the same host
have no say of their own.
"""

from __future__ import annotations

import logging
from typing import Any

log = logging.getLogger(__name__)


def approve_everywhere(platform: str, device_id: str, user_name: str,
                       stores: dict[str, Any] | None, adapter_name: str = "Herpots") -> list[str]:
    """Mirror ``device_id``'s approval into every non-default profile's pairing store.

    ``stores`` is the gateway runner's per-profile map (``pairing_stores``); the default profile's
    own store is skipped, it already gave the verdict. Returns the profiles the grant was added
    to. A store that cannot be written is logged and left to the next attempt; the others still
    get mirrored.
    """
    mirrored: list[str] = []
    for name, store in (stores or {}).items():
        if not name or name == "default" or store is None:
            continue
        try:
            with store._locked():
                if store.is_approved(platform, device_id):
                    continue
                store._approve_user(platform, device_id, user_name)
            mirrored.append(name)
        except Exception:
            log.warning("[%s] cannot mirror %s's approval into profile %s",
                        adapter_name, device_id, name, exc_info=True)
    if mirrored:
        log.info("[%s] %s approved for profiles: %s", adapter_name, device_id, ", ".join(mirrored))
    return mirrored
