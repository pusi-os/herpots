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

void set_storage(hg::Storage* store) { potsy_prefs::use_storage(store); }

bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms) {
  // The port draws the DIGITAL rain; the other Settings values map onto it
  // (BLACK would be the plain fill the caller does anyway).
  if (Settings::background() != Settings::Background::DIGITAL) return false;
  return paint_rain(c, x, y, w, h, now_ms);
}

void cycle_background() {
  const uint8_t next = (static_cast<uint8_t>(Settings::background()) + 1) % Settings::BACKGROUND_COUNT;
  Settings::setBackground(next);
}

void select_background(uint8_t value) { Settings::setBackground(value); }

bool background_is_rain() { return Settings::background() == Settings::Background::DIGITAL; }

void cycle_outfit(bool forward) {
  if (forward) Potsy::cycleOutfit();
  else Potsy::cyclePrevOutfit();
}

void select_outfit(uint8_t index) { Potsy::selectOutfit(index); }

const char* outfit_label() { return Potsy::outfitName(); }

void draw_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms, bool talking) {
  static uint32_t last_now = 0;

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

}  // namespace potsy_glue
