// Potsy port compat: the frame sprite. There is no TFT_eSprite here — the
// shim draws straight into the hg::Canvas — so the fast-sprite fast path is
// permanently off: g_frameSprite stays nullptr and potsy.cpp takes its
// library path.
#pragma once

class FastSprite;
// nullptr in this port (see potsy.cpp's `(g_frameSprite && ...) == &t` check).
extern FastSprite* g_frameSprite;
