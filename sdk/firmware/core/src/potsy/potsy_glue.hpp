// Potsy port: the glue between the hg renderer (Ui::draw_hero) and the
// PotsCharm mascot. One call site, no potsy headers in ui.cpp.
#pragma once

#include <cstdint>
#include "hg/canvas.hpp"

namespace potsy_glue {

// Draws Potsy into `c` (full-coordinate canvas; whatever lands outside the
// current band's clip rows is dropped) and advances his idle animation.
// `talking` opens his mouth while audio plays. His speech bubble carries only
// his own idle chatter; recognized requests and agent replies live in the
// caption band below him, never in the bubble.
void draw_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms, bool talking);

}  // namespace potsy_glue
