"""``persona``: the look one Hermes profile's devices draw, in ``persona.json``.

The record is what the ``persona`` wire message carries: the outfit, the backdrop, and the
character's own behaviour (chatter, animation tempo, whether he stands on the clock face).
"""

from __future__ import annotations

import json

import pytest

from hermes_gadget_plugin import persona


def test_a_profile_without_a_record_reads_as_the_defaults(tmp_path):
    assert persona.settings(tmp_path, "default") == persona.DEFAULTS
    (tmp_path / persona.FILENAME).write_text("{not json")
    assert persona.settings(tmp_path, "default") == persona.DEFAULTS
    (tmp_path / persona.FILENAME).write_text(json.dumps({"default": "nonsense"}))
    assert persona.settings(tmp_path, "default") == persona.DEFAULTS


def test_a_saved_record_round_trips_every_field(tmp_path):
    saved = persona.save(tmp_path, "work", outfit=4, background=2, banter=2, tempo=140, mini=False)
    assert saved == {"outfit": 4, "background": 2, "banter": 2, "tempo": 140, "mini": False}
    assert persona.settings(tmp_path, "work") == saved
    assert persona.all(tmp_path) == {"work": saved}
    assert not list(tmp_path.glob("*.tmp"))


def test_a_field_left_out_keeps_what_the_profile_had(tmp_path):
    persona.save(tmp_path, "work", outfit=4, background=2, banter=1, tempo=120, mini=True)
    assert persona.save(tmp_path, "work", outfit=7)["background"] == 2
    assert persona.save(tmp_path, "work", banter=0)["tempo"] == 120
    assert persona.settings(tmp_path, "work")["mini"] is True


def test_values_outside_what_the_firmware_draws_read_as_the_default(tmp_path):
    # An older or hand-edited file: an unknown value must not reach the device, which would
    # index past its own tables.
    (tmp_path / persona.FILENAME).write_text(json.dumps(
        {"default": {"outfit": 99, "background": -1, "banter": 5, "tempo": 5000, "mini": "yes"}}))
    assert persona.settings(tmp_path, "default") == persona.DEFAULTS


def test_save_refuses_a_value_the_firmware_would_not_take(tmp_path):
    for field, bad in (("outfit", persona.OUTFITS), ("outfit", -1), ("background", persona.BACKGROUNDS),
                       ("banter", 3), ("tempo", 49), ("tempo", 201), ("outfit", True), ("tempo", 1.5)):
        with pytest.raises(ValueError):
            persona.save(tmp_path, "default", **{field: bad})


def test_save_refuses_a_profile_name_the_file_cannot_hold(tmp_path):
    for name in ("", "   ", None, "x" * (persona.MAX_PROFILE + 1)):
        with pytest.raises(ValueError):
            persona.save(tmp_path, name, outfit=1)


def test_a_stale_other_profile_record_survives_a_write(tmp_path):
    persona.save(tmp_path, "work", outfit=1)
    persona.save(tmp_path, "home", outfit=2)
    assert persona.all(tmp_path) == {"home": {**persona.DEFAULTS, "outfit": 2},
                                     "work": {**persona.DEFAULTS, "outfit": 1}}
