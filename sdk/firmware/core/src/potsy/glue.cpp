// Potsy port: glue implementation. PotsCharm's mascot is drawn through the
// TFT_eSPI shim; the only state held here is the last bubble text (so the
// reply is announced once, not re-announced every frame) and the `advance`
// guard, which keeps his state machine advancing exactly once per frame even
// when a banded framebuffer calls draw_hero once per window.
#include "potsy_glue.hpp"

#include "potsy.h"
#include "TFT_eSPI.h"
#include "fast_sprite.h"

#include <string>

// The frame sprite fast path is off in this port (see fast_sprite.h).
FastSprite* g_frameSprite = nullptr;

namespace potsy_glue {

void draw_hero(hg::Canvas& c, int cx, int top_y, int avail_h, uint32_t now_ms, bool talking,
               const char* bubble) {
  static std::string last_bubble;
  static uint32_t announced_at = 0;
  static uint32_t last_now = 0;

  // tick() may be called several times per logical frame (one per framebuffer
  // window): state mutation only on the first call with a new `now`.
  const bool advance = now_ms != last_now;
  last_now = now_ms;

  TFT_eSPI t(&c);
  Potsy::setExternalTalking(talking);
  const char* text = bubble && bubble[0] ? bubble : nullptr;
  if (text) {
    // Announce on change; a long-lived identical reply is re-announced past
    // the bubble's six-second lifetime so it never sits there mute.
    const bool fresh = text != last_bubble || now_ms - announced_at > 7000;
    if (fresh) {
      Potsy::announce(text);
      last_bubble = text;
      announced_at = now_ms;
    }
    Potsy::holdBubble(false);
  } else if (!last_bubble.empty()) {
    // The reply went away: hold his bubble off the screen rather than let the
    // six-second lifetime run out over a screen that has moved on.
    last_bubble.clear();
    Potsy::holdBubble(true);
  }
  Potsy::tick(t, cx, top_y, avail_h, now_ms, advance, 0.5f);
}

}  // namespace potsy_glue
