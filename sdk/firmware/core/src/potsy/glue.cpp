// Potsy port: glue implementation. The only state held here is the `advance`
// guard, which keeps his state machine advancing exactly once per frame even
// when a banded framebuffer calls draw_hero once per window.
#include "potsy_glue.hpp"

#include "potsy.h"
#include "settings.h"
#include "TFT_eSPI.h"
#include "fast_sprite.h"

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


// A distinguishable solid for every non-DIGITAL value: eleven identical
// fills read as a stuck switch (the old behaviour), and dark tints keep the
// band's white text readable.
uint16_t solid_background() {
  switch (Settings::background()) {
    case Settings::Background::STARFIELD: return hg::rgb565(11, 16, 38);
    case Settings::Background::TOASTERS:  return hg::rgb565(42, 22, 8);
    case Settings::Background::AQUARIUM:  return hg::rgb565(6, 40, 46);
    case Settings::Background::TERMINAL:  return hg::rgb565(4, 20, 10);
    case Settings::Background::FIREFLIES: return hg::rgb565(12, 30, 16);
    case Settings::Background::FIRE:      return hg::rgb565(28, 6, 2);
    case Settings::Background::SNOWFALL:  return hg::rgb565(16, 28, 44);
    case Settings::Background::SPECTRUM:  return hg::rgb565(20, 10, 38);
    case Settings::Background::TUNNEL:    return hg::rgb565(20, 20, 20);
    case Settings::Background::SYNTHWAVE: return hg::rgb565(32, 10, 38);
    default:                              return hg::rgb565(0, 0, 0);
  }
}

bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms) {
  // DIGITAL is the rain; the other values are dark solids, one per value.
  if (Settings::background() == Settings::Background::DIGITAL)
    return paint_rain(c, x, y, w, h, now_ms);
  c.fill_rect(x, y, w, h, solid_background());
  return true;
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

void draw_mini_hero(hg::Canvas& c, int cx, int base_y, uint32_t now_ms, float scale) {
  TFT_eSPI t(&c);
  // A standing cameo, no bubble, no wave machine: the clock wants him calm.
  Potsy::drawWaving(t, cx, base_y, now_ms, scale, nullptr, false, 0, /*waving=*/false);
}

}  // namespace potsy_glue