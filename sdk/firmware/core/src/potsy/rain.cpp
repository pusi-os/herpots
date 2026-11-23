// The DIGITAL rain — the PotsCharm/vibe-pet backdrop, ported from the
// HerCharm M11 renderer (GPL-3.0, this project's own code). A pure function
// of time: every "random" choice is an integer hash of an index and the
// clock, so there is no per-column state, no allocation and no tearing with
// the banded framebuffer. Draws behind the character on the hero screen.
#include "potsy_glue.hpp"

#include <cstdint>

namespace potsy_glue {

namespace {

constexpr int COL_W = 12;   // 20 columns across a 240 px panel
constexpr int CELL = 10;    // one symbol cell, px
constexpr int GLYPH_W = 7;  // strokes stay inside a column
constexpr int GLYPH_H = 8;
constexpr int TRAIL = 8;    // lit cells behind a head (the longest)
constexpr int BANDS = 8;    // the night sky, in strips
// A symbol is reshuffled 6 times a second, not every frame: churn at the
// frame rate reads as noise, not as falling code.
constexpr uint32_t GLYPH_MS = 160;

inline uint32_t mix(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352du;
  x ^= x >> 15;
  x *= 0x846ca68bu;
  x ^= x >> 16;
  return x;
}

constexpr uint16_t kSkyTop = hg::rgb565(16, 24, 38);
constexpr uint16_t kDeep = hg::rgb565(10, 14, 20);
constexpr uint16_t kCyan = hg::rgb565(52, 196, 212);
constexpr uint16_t kGreen = hg::rgb565(38, 178, 96);

inline uint16_t blend(uint16_t a, uint16_t b, int num, int den) {
  const int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
  const int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
  return static_cast<uint16_t>(hg::rgb565(ar + (br - ar) * num / den,
                                          ag + (bg - ag) * num / den,
                                          ab + (bb - ab) * num / den));
}

struct Column {
  uint16_t speed;  // px per second
  uint16_t phase;  // start offset, px
  uint8_t len;     // lit cells
};

Column columnFor(int index) {
  const uint32_t h = mix(static_cast<uint32_t>(index) * 0x9e3779b9u + 0x85ebca6bu);
  Column c;
  c.speed = static_cast<uint16_t>(70 + (h % 5) * 25);  // 70..170 px/s
  c.phase = static_cast<uint16_t>((h >> 8) % 400u);
  c.len = static_cast<uint8_t>(5 + ((h >> 16) % 4));   // 5..8 cells
  return c;
}

// Head to tail: white-cyan, cyan, bright green, then greens fading into the
// sky. The rain sits on the app's near-black background, so the fade targets
// that instead of a themed sky.
const uint16_t* fadeTable() {
  static uint16_t fade[TRAIL];
  static bool ready = false;
  if (!ready) {
    for (int i = 0; i < TRAIL; i++) {
      fade[i] = i == 0 ? blend(kCyan, kDeep, 1, 2) : blend(kGreen, kDeep, TRAIL - 1 - i, TRAIL - 1);
    }
    ready = true;
  }
  return fade;
}

void drawGlyph(hg::Canvas& c, int x, int y, uint32_t seed, uint16_t colour) {
  c.fill_rect(x, y, 3 + static_cast<int>(seed % 3), 1, colour);
  c.fill_rect(x + static_cast<int>((seed >> 3) & 1), y + 3, 3 + static_cast<int>((seed >> 5) % 3), 1, colour);
  c.fill_rect(x, y + 6, 2 + static_cast<int>((seed >> 9) % 4), 1, colour);
  if (seed & 0x10000u) c.pixel(x + GLYPH_W - 1, y + 4, colour);
}

}  // namespace

bool paint_rain(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms) {
  if (w <= 0 || h <= 0) return false;
  // A night sky in 8 strips, a hair greener at the bottom, plus a 12 px glow
  // at the floor the rain lands on (the character stands just above it).
  for (int b = 0; b < BANDS; b++) {
    const int band_y = y + b * h / BANDS;
    const int band_h = h / BANDS + (b == BANDS - 1 ? h % BANDS : 0);
    c.fill_rect(x, band_y, w, band_h, blend(kDeep, kSkyTop, BANDS - 1 - b, BANDS - 1));
  }
  c.fill_rect(x, y + h - 12, w, 12, blend(kDeep, kGreen, 1, 10));

  const uint16_t* fade = fadeTable();
  // 65 s of clock keeps the speed multiply inside 32 bits (millis() wraps
  // after ~10 h and the phase would jump).
  const uint32_t clock = now_ms % 65536u;
  const uint32_t glyph_tick = now_ms / GLYPH_MS;
  const int cols = w / COL_W;
  for (int col = 0; col < cols; col++) {
    const Column ccol = columnFor(col);
    const uint32_t travel = static_cast<uint32_t>(h + ccol.len * CELL);
    const uint32_t head = (clock * ccol.speed / 1000u + ccol.phase) % travel;
    for (int i = 0; i < static_cast<int>(ccol.len); i++) {
      const int glyph_y = y + static_cast<int>(head) - i * CELL;
      if (glyph_y <= y - GLYPH_H || glyph_y >= y + h) continue;
      // A cell keeps its shape while it falls; only the freshly written head
      // is new. Seed is a function of the column, the cell and the tick.
      const uint32_t seed = mix(static_cast<uint32_t>(col) * 0x9e3779b9u ^
                                (head / CELL + static_cast<uint32_t>(i)) * 0x85ebca6bu ^ glyph_tick);
      drawGlyph(c, x + col * COL_W + 2, glyph_y, seed, fade[i]);
    }
  }
  return true;
}

}  // namespace potsy_glue
