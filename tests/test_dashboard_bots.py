"""The dashboard's session rows: each names the bot (Hermes profile) that runs it, and every
offered bot gets a row even with nothing running — the device switches bots from this list."""

from __future__ import annotations

from hermes_gadget_plugin import dashboard


def _board(profile: str, sessions: dict[str, str]) -> dashboard.SessionBoard:
    b = dashboard.SessionBoard(clock=lambda: 100.0)
    b.profile_name = profile
    for sid, state in sessions.items():
        b._set(sid, "herpots", state=state)
    return b


def test_a_session_row_names_the_bot_that_runs_it():
    board = _board("designer", {"aaaa1111": "idle"})
    rows = board.sessions()
    assert len(rows) == 1
    assert rows[0]["profile"] == "designer"
    assert rows[0]["state"] == "idle"
    assert rows[0]["id"] == "1111"


def test_other_boards_rows_keep_their_own_bot():
    mine = _board("default", {"aaaa1111": "idle"})
    theirs = _board("coder", {"bbbb2222": "busy"})
    rows = mine.sessions(theirs)
    by_profile = {r["profile"]: r for r in rows}
    assert set(by_profile) == {"default", "coder"}
    assert by_profile["coder"]["state"] == "busy"


def test_every_offered_bot_gets_a_row_when_nothing_runs_there():
    board = _board("default", {"aaaa1111": "idle"})
    roster = [{"id": "default", "name": "Герман"}, {"id": "coder", "name": "Кодер"}]
    rows = dashboard.with_bots(board.sessions(), roster)
    assert [r["profile"] for r in rows] == ["default", "coder"]  # the idle one kept once, not twice
    assert rows[1]["state"] == "idle"
    assert rows[1]["label"] == "Кодер"


def test_a_bot_with_a_live_session_is_not_added_twice():
    board = _board("coder", {"bbbb2222": "busy"})
    roster = [{"id": "coder", "name": "Кодер"}]
    rows = dashboard.with_bots(board.sessions(), roster)
    assert len(rows) == 1
    assert rows[0]["state"] == "busy"
