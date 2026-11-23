// Potsy port: glue implementation. The only state held here is the `advance`
// guard, which keeps his state machine advancing exactly once per frame even
// when a banded framebuffer calls draw_hero once per window.
#include "potsy_glue.hpp"

#include "potsy.h"
#include "TFT_eSPI.h"
#include "fast_sprite.h"

// The frame sprite fast path is off in this port (see fast_sprite.h).
FastSprite* g_frameSprite = nullptr;

namespace potsy_glue {

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
