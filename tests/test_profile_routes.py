"""The device's route in the gateway's live profile_routes table: the multiplexed gateway admits a
device request under the profile a ROUTE names, so switching must rewrite that route — otherwise
the first request under the new bot dies as profile_mismatch before it reaches any agent.
"""

from __future__ import annotations

from dataclasses import dataclass

from hermes_gadget_plugin import profile_routes


@dataclass(frozen=True)
class Route:
    name: str
    platform: str
    profile: str
    chat_id: str | None = None
    enabled: bool = True


def factory(**kw):
    return Route(**kw)


def test_a_switch_names_the_new_bot():
    got = profile_routes.sync_routes([], "herpots", "hg-1", "pots", factory)
    assert len(got) == 1
    assert got[0].profile == "pots" and got[0].chat_id == "hg-1"


def test_the_old_route_is_replaced_not_piled_up():
    first = profile_routes.sync_routes([], "herpots", "hg-1", "pots", factory)
    second = profile_routes.sync_routes(first, "herpots", "hg-1", "sem", factory)
    assert [r.profile for r in second] == ["sem"]


def test_back_to_default_removes_the_route():
    first = profile_routes.sync_routes([], "herpots", "hg-1", "pots", factory)
    second = profile_routes.sync_routes(first, "herpots", "hg-1", "default", factory)
    assert second == []


def test_the_operators_static_routes_survive():
    static = Route(name="shift", platform="herpots", profile="coder", chat_id="hg-9")
    got = profile_routes.sync_routes([static], "herpots", "hg-1", "pots", factory)
    assert static in got and len(got) == 2


def test_another_device_route_is_not_touched():
    other = Route(name=f"{profile_routes.PREFIX}pots", platform="herpots", profile="pots", chat_id="hg-2")
    got = profile_routes.sync_routes([other], "herpots", "hg-1", "sem", factory)
    assert other in got and [r for r in got if r.chat_id == "hg-1"][0].profile == "sem"


def test_an_operator_route_pins_the_device():
    static = Route(name="shift", platform="herpots", profile="coder", chat_id="hg-1")
    assert profile_routes.operator_routed([static], "herpots", "hg-1")
    # Our own route is the device's switching itself, never a pin.
    ours = Route(name=f"{profile_routes.PREFIX}pots", platform="herpots", profile="pots", chat_id="hg-1")
    assert not profile_routes.operator_routed([ours], "herpots", "hg-1")
    # A platform-wide route pins every device.
    wide = Route(name="all", platform="herpots", profile="coder", chat_id=None)
    assert profile_routes.operator_routed([wide], "herpots", "hg-1")
    # Other platforms and disabled routes do not.
    assert not profile_routes.operator_routed([Route(name="x", platform="telegram", profile="t")], "herpots", "hg-1")
    assert not profile_routes.operator_routed([Route(name="x", platform="herpots", profile="c", enabled=False)],
                                              "herpots", "hg-1")
