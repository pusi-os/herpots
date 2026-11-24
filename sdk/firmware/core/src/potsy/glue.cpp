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

bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms) {
  // DIGITAL is the rain (rain.cpp); the other values are their own animated
  // scenes. BLACK stays black — that is what the row promises.
  using Bg = Settings::Background;
  switch (Settings::background()) {
    case Bg::DIGITAL:   return paint_rain(c, x, y, w, h, now_ms);
    case Bg::STARFIELD: paint_starfield(c, x, y, w, h, now_ms); return true;
    case Bg::TOASTERS:  paint_toasters(c, x, y, w, h, now_ms);  return true;
    case Bg::FIREFLIES: paint_fireflies(c, x, y, w, h, now_ms); return true;
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

// The background's own name, for the settings row: the fills look alike,
// so the row body must name the value.
const char* background_label() {
  const bool ru = Settings::lang() != 0;
  switch (Settings::background()) {
    case Settings::Background::DIGITAL:   return ru ? "Цифровой дождь" : "Digital rain";
    case Settings::Background::STARFIELD: return ru ? "Звёздное небо" : "Starfield";
    case Settings::Background::TOASTERS:  return ru ? "Тостеры" : "Toasters";
    case Settings::Background::FIREFLIES: return ru ? "Светлячки" : "Fireflies";
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