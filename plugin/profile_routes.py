"""How a device's requests run under the bot it switched to.

A multiplexed gateway admits a primary adapter's message under the profile a **route** names
(``gateway.profile_routes``): ``session_ingress_context._binding`` resolves the runtime profile
from that table alone — a ``source.profile`` the adapter stamps itself is ignored (unless the
platform is a prefix producer, which herpots is not), and the turn then fails with
``profile_mismatch``. That is why switching looked broken: the switch itself succeeded, the very
first request under the new bot was refused before it reached any agent.

The device's bot is runtime state (it switches from its dashboard), so the plugin keeps ONE route
per device in the gateway's live table and rewrites it on every switch. Routes of ours carry a
name prefix: an operator's static route still pins a device (and blocks its switching), our own
never do.
"""

from __future__ import annotations

from typing import Any, Callable, Optional

PREFIX = "herpots-device:"


def device_route(routes: list[Any], device_id: str) -> Optional[Any]:
    """The route this plugin manages for ``device_id``, or None."""
    for route in routes or []:
        name = str(getattr(route, "name", ""))
        if name.startswith(PREFIX) and getattr(route, "chat_id", None) == device_id:
            return route
    return None


def sync_routes(routes: list[Any], platform: str, device_id: str, profile: str,
                route_factory: Callable[..., Any]) -> list[Any]:
    """The route table after this device moved to ``profile``.

    Replaces the device's managed route with one naming the new bot; a return to the default bot
    removes it (the default needs no route: the primary owner's own scope admits it). Other
    entries — the operator's static ones included — are kept untouched.
    """
    kept = [r for r in (routes or [])
            if not (str(getattr(r, "name", "")).startswith(PREFIX)
                    and getattr(r, "chat_id", None) == device_id)]
    if profile and profile != "default":
        kept.append(route_factory(name=f"{PREFIX}{profile}", platform=platform,
                                  profile=profile, chat_id=device_id))
    return kept


def operator_routed(routes: list[Any], platform: str, device_id: str) -> bool:
    """True when an operator's static route pins this device: the device may not choose.

    Our own managed routes are the device's switching itself, never a pin. A platform-wide route
    (no chat_id) pins every device on the platform.
    """
    for route in routes or []:
        if str(getattr(route, "name", "")).startswith(PREFIX):
            continue
        if getattr(route, "enabled", True) is False:
            continue
        if getattr(route, "platform", None) != platform:
            continue
        chat = getattr(route, "chat_id", None)
        if chat is None or chat == device_id:
            return True
    return False
