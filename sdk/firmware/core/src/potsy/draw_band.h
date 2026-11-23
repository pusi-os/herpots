// Potsy port compat: which rows the current render pass can paint. The
// hg renderer already clips at the Canvas level, so every query answers
// "whole screen" (the non-CYD35 build of the original header).
#pragma once

namespace DrawBand {

inline void set(int, int) {}
inline void all()         {}
inline bool has(int, int) { return true; }
inline int  top(int y)    { return y; }
inline int  bot(int y)    { return y; }
inline void setEnabled(bool) {}
inline bool enabled()        { return false; }

}  // namespace DrawBand
