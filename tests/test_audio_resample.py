"""Speaker audio: the resampler feeding a 16 kHz gadget from a 24/44.1 kHz source.

Crackle over speech is what aliasing sounds like, so the property under test is that a tone
above the destination Nyquist does not fold back into the audible band as a strong tone.
"""

from __future__ import annotations

import math
from array import array

import pytest

from hermes_gadget_plugin import audio


def _tone(freq: float, rate: int, seconds: float, amp: int = 12000) -> bytes:
    n = int(rate * seconds)
    samples = array("h", (int(amp * math.sin(2 * math.pi * freq * i / rate)) for i in range(n)))
    return samples.tobytes()


def _peak_at(pcm: bytes, rate: int, freq: float) -> float:
    """Magnitude of ``freq`` in the signal, by a single-bin DFT (normalised to sample count)."""
    s = array("h")
    s.frombytes(pcm[: len(pcm) - len(pcm) % 2])
    n = len(s)
    if n == 0:
        return 0.0
    re = sum(v * math.cos(2 * math.pi * freq * i / rate) for i, v in enumerate(s))
    im = sum(v * math.sin(2 * math.pi * freq * i / rate) for i, v in enumerate(s))
    return math.hypot(re, im) / n


def test_a_tone_inside_the_band_survives_resampling():
    src = _tone(440.0, 24000, 0.2)
    out = audio.resample(src, 24000, 16000)
    assert abs(len(out) / 2 / 16000 - 0.2) < 0.01  # the same duration
    assert _peak_at(out, 16000, 440.0) > 12000 * 0.35  # still loud


def test_a_tone_above_the_destination_nyquist_does_not_fold_back():
    # 10 kHz is above 8 kHz (the 16 kHz Nyquist rate). Without filtering it lands at 6 kHz.
    src = _tone(10000.0, 24000, 0.2)
    out = audio.resample(src, 24000, 16000)
    folded = _peak_at(out, 16000, 6000.0)
    assert folded < 12000 * 0.25, f"10 kHz folded to 6 kHz at {folded:.0f}: the resampler is not filtering"
