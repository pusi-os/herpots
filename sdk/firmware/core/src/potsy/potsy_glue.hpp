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
// rain, the other values are distinguishable dark solids. The rain is a pure
// function of `now_ms`, so banded framebuffer redraws agree.
bool paint_background(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms);

// A small standing cameo for screens that want him without the state
// machine (the clock face's avatar slot): feet on `base_y`, height about
// 58*scale px, idle bob only, no speech bubble.
void draw_mini_hero(hg::Canvas& c, int cx, int base_y, uint32_t now_ms, float scale);

// rain.cpp: the rain renderer paint_background() delegates to.
bool paint_rain(hg::Canvas& c, int x, int y, int w, int h, uint32_t now_ms);

// The runtime background (the persona message and the settings menu): cycles
// forward through the values or sets one directly. Both persist.
void cycle_background();
void select_background(uint8_t value);
// True while the drawn backdrop is the DIGITAL rain (the settings row's body).
bool background_is_rain();

// Outfit controls for the settings menu and the persona message. Both cycle
// directions step only through unlocked outfits; select_outfit unlocks the
// set if the owner picks a locked one. `outfit_label()` names the current one.
void cycle_outfit(bool forward);
void select_outfit(uint8_t index);
const char* outfit_label();

// The current background's name (the settings row's body): one label per value.
const char* background_label();

}  // namespace potsy_glue
