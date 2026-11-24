"""One pairing is every bot's: an approval on the default profile is mirrored into each served
profile's pairing store, so a device switched to another bot runs there without a second pairing.

This is what made session switching look broken: the switch itself worked, but the first request
under the new bot met that profile's own pairing gate, and its code came back as a chat message
instead of a panel request.
"""

from __future__ import annotations

import threading

from hermes_gadget_plugin import pairing_mirror

PLATFORM = "herpots"


class _ctx:
    def __init__(self, lock):
        self._lock = lock

    def __enter__(self):
        self._lock.acquire()

    def __exit__(self, *exc):
        self._lock.release()


class FakeStore:
    """The bits of gateway.pairing.PairingStore the mirroring touches."""

    def __init__(self, approved: set[str] | None = None):
        self._approved = approved or set()
        self._lock = threading.RLock()
        self.named: list[tuple[str, str]] = []

    def _locked(self):
        return _ctx(self._lock)

    def is_approved(self, platform: str, user_id: str) -> bool:
        assert platform == PLATFORM
        return user_id in self._approved

    def _approve_user(self, platform: str, user_id: str, user_name: str = "") -> None:
        assert platform == PLATFORM
        self._approved.add(user_id)
        self.named.append((user_id, user_name))


def test_an_approval_reaches_every_other_profile():
    teamlead, coder = FakeStore(), FakeStore()
    got = pairing_mirror.approve_everywhere(
        PLATFORM, "hg-1", "HerPots",
        {"default": FakeStore({"hg-1"}), "teamlead": teamlead, "coder": coder})
    assert got == ["teamlead", "coder"]
    assert teamlead.is_approved(PLATFORM, "hg-1") and coder.is_approved(PLATFORM, "hg-1")
    assert teamlead.named == [("hg-1", "HerPots")]  # the device's name goes along


def test_the_default_profile_is_not_touched():
    default = FakeStore({"hg-1"})
    pairing_mirror.approve_everywhere(PLATFORM, "hg-1", "HerPots", {"default": default})
    assert default.named == []  # its own verdict stays the single source


def test_a_profile_that_already_approves_is_left_alone():
    teamlead = FakeStore({"hg-1"})
    got = pairing_mirror.approve_everywhere(
        PLATFORM, "hg-1", "HerPots", {"default": FakeStore({"hg-1"}), "teamlead": teamlead})
    assert got == []
    assert teamlead.named == []  # no second write over an existing grant


def test_a_broken_store_does_not_stop_the_others():
    class Broken(FakeStore):
        def _approve_user(self, *a, **kw):
            raise OSError("disk gone")

    teamlead, coder = Broken(), FakeStore()
    got = pairing_mirror.approve_everywhere(
        PLATFORM, "hg-1", "HerPots",
        {"default": FakeStore({"hg-1"}), "teamlead": teamlead, "coder": coder})
    assert got == ["coder"]  # the broken one was skipped, not the whole mirroring
    assert coder.is_approved(PLATFORM, "hg-1")


def test_without_stores_it_is_a_quiet_no_op():
    assert pairing_mirror.approve_everywhere(PLATFORM, "hg-1", "HerPots", None) == []
