// Potsy port: the glue between the hg renderer (Ui::draw_hero) and the
// PotsCharm mascot. One call site, no potsy headers in ui.cpp.
#pragma once

#include <cstdint>
#include "hg/canvas.hpp"
#include "hg/hal.hpp"

namespace potsy_glue {

// Registers the port's persistent store: Potsy's cosmetics namespace and the
// runtime background persist through it. Call once from the app at boot.
void set_storage(hg::Storage* store);

// Draws Potsy into `c` (full-coordinate canvas; whatever lands outside the
// current band's clip rows is dropped) and advances his idle animation.
// `talking` opens his mouth while audio plays. His speech bubble carries only
// his own idle chatter; recognized requests and agent replies live in the
// caption band below him, never in the bubble.
void draw_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms, bool talking);

// Paints the hero band's backdrop. Always true (it drew): DIGITAL is the
// rain, the other values are their own animated scenes (the paint_* renderers
// in glue.cpp, all pure functions of now_ms). The rain is a pure function of
// `now_ms`, so banded framebuffer redraws agree.
bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms);

// The clock face's avatar slot: a full tick() of the same Potsy the hero band
// draws — the whole pose machine, bouncing and reacting — with his speech
// bubble held off so his idle chatter never covers the time and the date.
// `top_y`/`avail_h` are the slot's band: he sizes himself to fill it.
void draw_mini_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms);

// rain.cpp: the rain renderer paint_background() delegates to.
bool paint_rain(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms);

// The runtime background (the persona message and the settings menu): cycles
// forward through the values or sets one directly. Both persist.
void cycle_background();
void select_background(uint8_t value);
// True while the drawn backdrop is the DIGITAL rain (the settings row's body).
bool background_is_rain();

// What the clock face reads to decide whether Potsy stands in its avatar slot
// (the persona's `mini`, default on). Needs no Potsy headers in ui.cpp.
bool mini_on_clock();

// Outfit controls for the settings menu and the persona message. Both cycle
// directions step only through unlocked outfits; select_outfit unlocks the
// set if the owner picks a locked one. `outfit_label()` names the current one.
void cycle_outfit(bool forward);
void select_outfit(uint8_t index);
const char* outfit_label();

// The character's own behaviour, set from the persona message or the settings
// menu: the idle-chatter level (0 quiet, 1 normal, 2 talkative), his animation
// tempo in percent (50..200, applied at once), and whether he also stands on
// the clock face. Each persists; both labels name the current value.
void select_banter(uint8_t level);
void select_tempo(uint8_t pct);
void select_mini(bool on);
const char* banter_label();
const char* mini_label();
uint8_t tempo_percent();

// The current background's name (the settings row's body): one label per value.
const char* background_label();

}  // namespace potsy_glue
