// Potsy port: glue implementation. The only state held here is the `advance`
// guard, which keeps his state machine advancing exactly once per frame even
// when a banded framebuffer calls draw_hero once per window.
#include "potsy_glue.hpp"

#include "potsy.h"
#include "settings.h"
#include "TFT_eSPI.h"
#include "fast_sprite.h"
#include <cmath>

// The frame sprite fast path is off in this port (see fast_sprite.h).
FastSprite* g_frameSprite = nullptr;

namespace potsy_glue {

// The fork's cosmetics are free from the first boot: the outfit cycle in the
// settings menu and the persona message from Hermes must be able to pick any
// costume. PotsCharm gates outfits behind lifetime stats; seeding the master
// unlock once, before the first prefs load, lifts that gate without touching
// the vendored potsy.cpp. Idempotent: written only while the key is absent.
void seed_unlock() {
  static bool seeded = false;
  if (!seeded) {
    seeded = true;
    if (!potsy_prefs::read("allOutfits")) potsy_prefs::write("allOutfits", "1");
  }
}

void set_storage(hg::Storage* store) {
  potsy_prefs::use_storage(store);
  // Before the first prefs load: Potsy's loadPrefs runs after App's constructor
  // (trigger BOOTED), so seeding here lands in RAM on the very first boot.
  seed_unlock();
}


// ---- Animated backgrounds ---------------------------------------------------
// Every renderer below is a pure function of (rect, now_ms): no state, no
// allocation, so banded framebuffer redraws agree and nothing tears across
// bands — the same contract rain.cpp documents. All of them keep the band's
// white text readable (dark fields), and every "random" choice is an integer
// hash of an index and the clock.

inline uint32_t mix32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352du;
  x ^= x >> 15;
  x *= 0x846ca68bu;
  x ^= x >> 16;
  return x;
}

inline uint16_t mix565(uint16_t a, uint16_t b, int num, int den) {
  const int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
  const int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
  return static_cast<uint16_t>(hg::rgb565(ar + (br - ar) * num / den,
                                          ag + (bg - ag) * num / den,
                                          ab + (bb - ab) * num / den));
}

// v wrapped into [0, m), negative-safe.
inline int wrap(int v, int m) {
  v %= m;
  return v < 0 ? v + m : v;
}

// STARFIELD: the night sky, stars drifting slowly left and twinkling.
void paint_starfield(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t sky = hg::rgb565(11, 16, 38);
  c.fill_rect(x, y, w, h, sky);
  for (int i = 0; i < 64; ++i) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(i) * 0x9e3779b9u);
    const int speed = 4 + static_cast<int>(hsh % 5);                    // px/s, drift left
    const int sx = x + wrap(static_cast<int>((hsh >> 8) & 1023) * w / 1024 -
                            static_cast<int>(now / 1000u) * speed, w);
    const int sy = y + static_cast<int>((hsh >> 20) & 1023) * h / 1024;
    const uint32_t tw = (now / 450u + (hsh >> 4)) % 7u;
    if (tw == 0) continue;                                              // the off phase
    const uint16_t col = mix565(sky, hg::rgb565(220, 228, 255),
                                static_cast<int>(tw) * 255 / 6, 255);
    if ((hsh & 0x40u) && tw > 4) c.fill_rect(sx, sy, 2, 2, col);        // the bright few
    else                                         c.pixel(sx, sy, col);
  }
}

// TOASTERS: the flock flying left to right, a slice toast breaking free of
// each one. Same characters the original background taught Potsy to duck.
void paint_toasters(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t sky    = hg::rgb565(42, 22, 8);
  const uint16_t shell  = hg::rgb565(148, 152, 160);
  const uint16_t shellD = hg::rgb565(88, 92, 100);
  const uint16_t slot   = hg::rgb565(24, 14, 6);
  const uint16_t crumb  = hg::rgb565(198, 142, 66);
  const uint16_t crust  = hg::rgb565(150, 96, 36);
  c.fill_rect(x, y, w, h, sky);
  for (int k = 0; k < 3; ++k) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(k) * 0x85ebca6bu + 0x9e3779b9u);
    const int speed = 26 + static_cast<int>(hsh % 3) * 10;              // px/s
    const int fy = y + h / 5 + static_cast<int>((hsh >> 8) % 3u) * h / 4;
    const int fx = x + wrap(static_cast<int>(now / 1000u) * speed +
                            static_cast<int>((hsh >> 16) & 255), w + 72) - 36;
    // The slice, above and behind the toaster it just left.
    c.fill_rect(fx - 14, fy - 12, 9, 11, crust);
    c.fill_rect(fx - 13, fy - 11, 7, 9, crumb);
    // The toaster: shell, slot, feet.
    c.fill_round_rect(fx, fy, 24, 15, 3, shell);
    c.fill_rect(fx + 4, fy + 3, 16, 2, slot);
  }
}

// AQUARIUM: teal water, bubbles on their way up, fish on their way across.
void paint_aquarium(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t top  = hg::rgb565(6, 40, 46);
  const uint16_t deep = hg::rgb565(4, 26, 32);
  for (int b = 0; b < 4; ++b)
    c.fill_rect(x, y + b * h / 4, w, h / 4 + (b == 3 ? h % 4 : 0),
                mix565(top, deep, 3 - b, 3));
  const uint16_t bubble = hg::rgb565(110, 190, 200);
  const uint16_t bubbleD = mix565(deep, bubble, 1, 3);
  for (int i = 0; i < 22; ++i) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(i) * 0x27220a95u);
    const int bx = x + static_cast<int>((hsh >> 4) & 1023) * w / 1024;
    const int speed = 14 + static_cast<int>(hsh % 3) * 6;               // px/s, rising
    const int by = y + h - wrap(static_cast<int>(now / 1000u) * speed +
                                static_cast<int>((hsh >> 16) & 255), h + 14) + 4;
    const int r = static_cast<int>(hsh & 1u) + 1;
    if (hsh & 0x80u) c.ring(bx, by, r + 1, 1, bubble);
    else             c.pixel(bx, by, bubbleD);
  }
  const uint16_t fish = hg::rgb565(232, 122, 52);
  for (int k = 0; k < 3; ++k) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(k) * 0x9e3779b9u + 0x165667b1u);
    const bool right = (hsh & 1u) != 0;
    const int dir = right ? 1 : -1;
    const int speed = 18 + static_cast<int>((hsh >> 4) % 3u) * 7;       // px/s
    const int fx = x + wrap(static_cast<int>(now / 1000u) * speed * dir +
                            static_cast<int>((hsh >> 12) & 255), w + 44) - 22;
    const int fy = y + h / 5 + static_cast<int>((hsh >> 8) % 3u) * h / 4 +
                   static_cast<int>(sinf(static_cast<float>(now / 650 + k * 40) * 0.017453f) * 4.0f);
    (void)dir;
    c.fill_circle(fx, fy, 4, fish);
    c.fill_rect(fx - dir * 6, fy - 2, 3, 4, fish);                      // the tail, behind
  }
}

// TERMINAL: a dim green console — blocky glyphs reshuffling on a grid, a
// sweep line walking down the pane, a cursor blinking in the corner.
void paint_terminal(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t bg     = hg::rgb565(4, 20, 10);
  const uint16_t bright = hg::rgb565(80, 230, 130);
  const uint16_t dim    = mix565(bg, bright, 1, 3);
  const uint16_t faint  = mix565(bg, bright, 1, 6);
  c.fill_rect(x, y, w, h, bg);
  const int cw = 8, ch = 7;
  const uint32_t tick = now / 300u;
  for (int ry = 0; ry < h / ch + 1; ++ry) {
    for (int rx = 0; rx < w / cw + 1; ++rx) {
      const uint32_t hsh = mix32(static_cast<uint32_t>(rx * 97 + ry * 131 + static_cast<int>(tick) * 7919));
      if (hsh % 5 == 0) continue;                                       // some cells sit empty
      const uint16_t col = (hsh & 3u) == 1 ? bright : ((hsh & 3u) == 2 ? dim : faint);
      const int gx = x + rx * cw, gy = y + ry * ch;
      c.fill_rect(gx, gy, 3 + static_cast<int>(hsh % 3), 2, col);
    }
  }
  const int sweep = y + wrap(static_cast<int>(now / 9), h);             // the refresh line
  c.fill_rect(x, sweep, w, 1, mix565(bg, bright, 1, 2));
  if ((now / 500u) & 1u) c.fill_rect(x + 2, y + h - ch - 1, cw - 3, ch - 2, bright);  // the cursor
}

// FIREFLIES: warm dots wandering calm Lissajous paths over the dark green.
void paint_fireflies(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t night = hg::rgb565(12, 30, 16);
  c.fill_rect(x, y, w, h, night);
  const uint16_t core = hg::rgb565(224, 255, 150);
  const uint16_t glow = hg::rgb565(96, 150, 60);
  for (int i = 0; i < 14; ++i) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(i) * 0x2545f491u);
    const float f1 = 0.00011f + static_cast<float>(hsh % 3u) * 0.00004f;
    const float f2 = 0.00013f + static_cast<float>((hsh >> 4) % 3u) * 0.00004f;
    const float p1 = static_cast<float>(hsh & 1023u) * 0.0061f;
    const float p2 = static_cast<float>((hsh >> 10) & 1023u) * 0.0061f;
    const int fx = x + w / 2 + static_cast<int>(sinf(static_cast<float>(now) * f1 + p1) * static_cast<float>(w) / 3.2f);
    const int fy = y + h / 2 + static_cast<int>(sinf(static_cast<float>(now) * f2 + p2) * static_cast<float>(h) / 3.2f);
    const bool lit = ((now / 700u) + static_cast<uint32_t>(i)) % 6u < 4u;
    if (!lit) continue;                                                 // most rest, some glow
    c.pixel(fx, fy, core);
    c.pixel(fx - 1, fy, glow); c.pixel(fx + 1, fy, glow);
    c.pixel(fx, fy - 1, glow); c.pixel(fx, fy + 1, glow);
  }
}

// FIRE: flames flickering along the floor, sparks drifting up off them.
void paint_fire(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t bg = hg::rgb565(28, 6, 2);
  c.fill_rect(x, y, w, h, bg);
  static const uint16_t tiers[3] = {hg::rgb565(252, 206, 64), hg::rgb565(240, 112, 22),
                                    hg::rgb565(168, 42, 10)};
  const int colw = 8;
  for (int col = 0; col < w / colw + 1; ++col) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(col) * 13u + now / 120u);
    int hf = h / 3 + static_cast<int>(hsh % 7u) * h / 28;               // this column's height
    for (int L = 0; L < 3 && hf > 0; ++L) {                             // yellow, orange, red
      const int seg = (hf + 2) / 3;
      c.fill_rect(x + col * colw, y + h - hf, colw, seg, tiers[L]);
      hf -= seg;
    }
  }
  const uint16_t ember = hg::rgb565(255, 130, 44);
  for (int i = 0; i < 20; ++i) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(i) * 0x9e3779b9u + 0x27d4eb2fu);
    const int speed = 20 + static_cast<int>(hsh % 3u) * 9;              // px/s, rising
    const int travel = h + 10;
    const int prog = wrap(static_cast<int>(now / 1000u) * speed +
                          static_cast<int>((hsh >> 8) & 255), travel);
    const int ey = y + h - prog;
    const int ex = x + static_cast<int>((hsh >> 16) & 1023) * w / 1024 +
                   static_cast<int>(sinf(static_cast<float>(now / 500 + i * 37) * 0.017453f) * 3.0f);
    if (prog < 6 || prog > travel - 4) continue;                        // born in, die out
    c.pixel(ex, ey, prog < travel / 3 ? ember : mix565(bg, ember, 1, 2));
  }
}

// SNOWFALL: flakes drifting down, swaying as they fall.
void paint_snowfall(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t sky = hg::rgb565(16, 28, 44);
  c.fill_rect(x, y, w, h, sky);
  const uint16_t flake = hg::rgb565(225, 235, 250);
  const uint16_t flakeD = mix565(sky, flake, 1, 2);
  for (int i = 0; i < 80; ++i) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(i) * 0x9e3779b9u + 0x68e31da4u);
    const int speed = 16 + static_cast<int>(hsh % 4u) * 9;              // px/s, falling
    const int fy = y + wrap(static_cast<int>(now / 1000u) * speed +
                            static_cast<int>((hsh >> 6) & 255), h + 8) - 4;
    const int fx = x + wrap(static_cast<int>((hsh >> 14) & 1023) * w / 1024 +
                            static_cast<int>(sinf(static_cast<float>(now / 900 + i * 29) * 0.017453f) * 7.0f), w);
    if (hsh & 0x40u) c.fill_rect(fx, fy, 2, 2, flake);
    else             c.pixel(fx, fy, flakeD);
  }
}

// SPECTRUM: an equalizer dancing on the beat, peak caps falling after it.
void paint_spectrum(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t bg = hg::rgb565(20, 10, 38);
  c.fill_rect(x, y, w, h, bg);
  const uint16_t green  = hg::rgb565(60, 220, 120);
  const uint16_t yellow = hg::rgb565(242, 222, 62);
  const uint16_t red    = hg::rgb565(240, 84, 52);
  const int bw = 12;
  const uint32_t beat = now / 140u;
  for (int b = 0; b < w / bw + 1; ++b) {
    const uint32_t hsh = mix32(static_cast<uint32_t>(b) * 761u + beat * 1013u);
    const int bh = 6 + static_cast<int>(hsh % 7u) * h / 10;             // this beat's height
    const int bx = x + b * bw;
    c.fill_rect(bx, y + h - bh, bw - 3, bh, bh > h / 3 ? yellow : green);
    if (bh > h / 3) c.fill_rect(bx, y + h - bh, bw - 3, 2, red);        // the hot top of the bar
  }
}

// TUNNEL: rings rushing out of a slowly wandering vanishing point.
void paint_tunnel(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t bg = hg::rgb565(20, 20, 20);
  c.fill_rect(x, y, w, h, bg);
  const int cx = x + w / 2 + static_cast<int>(sinf(static_cast<float>(now) * 0.00031f) * static_cast<float>(w) / 7.0f);
  const int cy = y + h / 2 + static_cast<int>(cosf(static_cast<float>(now) * 0.00023f) * static_cast<float>(h) / 7.0f);
  const int maxr = (w + h) / 3;
  for (int k = 0; k < 8; ++k) {
    const int r = wrap(static_cast<int>(now / 24) + k * (maxr / 8), maxr);
    if (r < 3) continue;
    c.ring(cx, cy, r, 2, mix565(bg, hg::rgb565(200, 200, 210), maxr - r, maxr));
  }
}

// SYNTHWAVE: a striped sun on the horizon and a perspective grid marching by.
void paint_synthwave(hg::Canvas& c, int x, int y, int w, int h, uint32_t now) {
  const uint16_t bg   = hg::rgb565(32, 10, 38);
  const uint16_t sun  = hg::rgb565(250, 122, 92);
  const uint16_t grid = hg::rgb565(238, 84, 158);
  c.fill_rect(x, y, w, h, bg);
  const int horizon = y + h / 2;
  const int scx = x + w / 2, scy = horizon, sr = w / 5;
  c.fill_circle(scx, scy, sr, sun);
  for (int k = 0; k < 4; ++k) {                                         // the sun's gap stripes
    c.fill_rect(scx - sr - 2, scy + (k + 1) * sr / 5, 2 * sr + 4, 1 + k, bg);
  }
  c.fill_rect(x, horizon, w, 1, grid);
  // Horizontal grid lines: quadratic spacing reads as perspective, the march
  // as motion towards the viewer.
  for (int k = 0; k < 6; ++k) {
    const float frac = wrap(static_cast<int>(now / 1300.0f * 100.0f) + k * 17, 100) / 100.0f;
    const int gy = horizon + 1 + static_cast<int>(static_cast<float>(y + h - horizon - 2) * frac * frac);
    c.fill_rect(x, gy, w, 1, mix565(bg, grid, 1 + static_cast<int>(frac * 3.0f), 4));
  }
}

bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms) {
  // DIGITAL is the rain (rain.cpp); the other values are their own animated
  // scenes. BLACK stays black — that is what the row promises.
  using Bg = Settings::Background;
  switch (Settings::background()) {
    case Bg::DIGITAL:   return paint_rain(c, x, y, w, h, now_ms);
    case Bg::STARFIELD: paint_starfield(c, x, y, w, h, now_ms); return true;
    case Bg::TOASTERS:  paint_toasters(c, x, y, w, h, now_ms);  return true;
    case Bg::AQUARIUM:  paint_aquarium(c, x, y, w, h, now_ms);  return true;
    case Bg::TERMINAL:  paint_terminal(c, x, y, w, h, now_ms);  return true;
    case Bg::FIREFLIES: paint_fireflies(c, x, y, w, h, now_ms); return true;
    case Bg::FIRE:      paint_fire(c, x, y, w, h, now_ms);      return true;
    case Bg::SNOWFALL:  paint_snowfall(c, x, y, w, h, now_ms);  return true;
    case Bg::SPECTRUM:  paint_spectrum(c, x, y, w, h, now_ms);  return true;
    case Bg::TUNNEL:    paint_tunnel(c, x, y, w, h, now_ms);    return true;
    case Bg::SYNTHWAVE: paint_synthwave(c, x, y, w, h, now_ms); return true;
    default:            c.fill_rect(x, y, w, h, hg::rgb565(0, 0, 0)); return true;
  }
}

void cycle_background() {
  const uint8_t next = (static_cast<uint8_t>(Settings::background()) + 1) % Settings::BACKGROUND_COUNT;
  Settings::setBackground(next);
}

void select_background(uint8_t value) { Settings::setBackground(value); }

bool background_is_rain() { return Settings::background() == Settings::Background::DIGITAL; }

void cycle_outfit(bool forward) {
  seed_unlock();
  if (forward) Potsy::cycleOutfit();
  else Potsy::cyclePrevOutfit();
}

void select_outfit(uint8_t index) { seed_unlock(); Potsy::selectOutfit(index); }

const char* outfit_label() { return Potsy::outfitName(); }

// The background's own name, for the settings row: all eleven non-rain values
// drew as one undistinguishable fill, so the row body must name the value.
const char* background_label() {
  const bool ru = Settings::lang() != 0;
  switch (Settings::background()) {
    case Settings::Background::DIGITAL:   return ru ? "Цифровой дождь" : "Digital rain";
    case Settings::Background::STARFIELD: return ru ? "Звёздное небо" : "Starfield";
    case Settings::Background::TOASTERS:  return ru ? "Тостеры" : "Toasters";
    case Settings::Background::AQUARIUM:  return ru ? "Аквариум" : "Aquarium";
    case Settings::Background::TERMINAL:  return ru ? "Терминал" : "Terminal";
    case Settings::Background::FIREFLIES: return ru ? "Светлячки" : "Fireflies";
    case Settings::Background::FIRE:      return ru ? "Огонь" : "Fire";
    case Settings::Background::SNOWFALL:  return ru ? "Снегопад" : "Snowfall";
    case Settings::Background::SPECTRUM:  return ru ? "Спектр" : "Spectrum";
    case Settings::Background::TUNNEL:    return ru ? "Тоннель" : "Tunnel";
    case Settings::Background::SYNTHWAVE: return ru ? "Синтвейв" : "Synthwave";
    default:                              return ru ? "Чёрный" : "Black";
  }
}

void draw_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms, bool talking) {
  static uint32_t last_now = 0;
  seed_unlock();

  // tick() may be called several times per logical frame (one per framebuffer
  // window): state mutation only on the first call with a new `now`.
  const bool advance = now_ms != last_now;
  last_now = now_ms;

  TFT_eSPI t(&c);
  Potsy::setExternalTalking(talking);
  // No external bubble feed on purpose: the recognized request and the agent's
  // reply are shown in the caption band below him, and feeding them here made
  // his bubble flicker with every streamed chunk. The bubble above his head
  // carries only his own idle chatter (his pools, picked by his state machine).
  Potsy::tick(t, cx, top_y, avail_h, now_ms, advance, 0.5f);
}

void draw_mini_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms) {
  static uint32_t last_now = 0;
  // The clock repaints every frame now that he is animated, but it still
  // renders through the banded framebuffer: state advances exactly once per
  // logical frame (same guard draw_hero() uses).
  const bool advance = now_ms != last_now;
  last_now = now_ms;
  TFT_eSPI t(&c);
  // A full tick() — the whole pose machine, so the clock cameo bounces,
  // stretches and reacts like the hero band's Potsy — with his speech
  // bubble held off: the clock wants him moving, not talking, and a
  // chatter bubble would sit right on top of the time and the date.
  Potsy::holdBubble(true);
  Potsy::tick(t, cx, top_y, avail_h, now_ms, advance, 0.5f,
              /*scanningFx=*/false, /*wanderRangePx=*/avail_h / 3);
  Potsy::holdBubble(false);
}

}  // namespace potsy_glue