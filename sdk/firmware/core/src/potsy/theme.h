// Potsy port compat: the slice of PotsCharm include/theme.h potsy.cpp needs.
// Fixed colours copied verbatim; the swappable palette is the default preset,
// never re-applied (no settings menu here). Fonts: the hg 5x7 face stands in
// for the GLCD/font2 pair — one text size, EN only.
#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "TFT_eSPI.h"
#include "state.h"

namespace Theme {

// ---- swappable palette, default preset ----
inline uint16_t BG           = 0x0801;  // #0a000f-ish
inline uint16_t TASKBAR      = 0x0803;
inline uint16_t PURPLE       = 0xAC1F;
inline uint16_t CYAN         = 0x07FF;
inline uint16_t PINK         = 0xF96F;
inline uint16_t VAPOR_PINK   = 0xFB99;
inline uint16_t VAPOR_PURPLE = 0xBB5F;
inline uint16_t VAPOR_BLUE   = 0x067F;
inline uint16_t VAPOR_YELLOW = 0xFFFB;
inline uint16_t GREEN        = 0x07E0;
inline uint16_t AMBER        = 0xFD20;
inline uint16_t RED          = 0xF800;

// Fixed regardless of theme.
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t BLACK = 0x0000;

// Potsy's skin — Simpson yellow.
constexpr uint16_t FUR_DARK  = 0x38C0;
constexpr uint16_t FUR_MAIN  = 0xFF2B;
constexpr uint16_t FUR_LIGHT = 0xFF2B;
constexpr uint16_t SKIN_TAN  = 0xFF2B;
constexpr uint16_t FUR_SHADE = 0xFEA3;
constexpr uint16_t SKIN_DARK = 0xCB88;

// Windows 95/98 system chrome (see the original header for the 565 ramp).
constexpr uint16_t W95_FACE     = 0xB5B6;
constexpr uint16_t W95_HILITE   = 0xFFFF;
constexpr uint16_t W95_LIGHT    = 0xDEFB;
constexpr uint16_t W95_SHADOW   = 0x8410;
constexpr uint16_t W95_DKSHADOW = 0x0000;

// ---- helpers -----------------------------------------------------------

inline bool large() { return false; }  // 240x240 is a small panel

// Linear blend between two RGB565 colors, t 0..256 (8.8 fixed).
inline uint16_t blend(uint16_t a, uint16_t b, uint16_t t) {
  t = std::min<uint16_t>(t, 256);
  auto ch = [](uint16_t c, int shift, int bits) {
    int mask = (1 << bits) - 1;
    return (c >> shift) & mask;
  };
  int r = (ch(a, 11, 5) * (256 - t) + ch(b, 11, 5) * t) / 256;
  int g = (ch(a, 5, 6) * (256 - t) + ch(b, 5, 6) * t) / 256;
  int bl = (ch(a, 0, 5) * (256 - t) + ch(b, 0, 5) * t) / 256;
  return static_cast<uint16_t>((r << 11) | (g << 5) | bl);
}

// Detection colours: one cyan for everything (v1 draws no dex/log UI).
inline uint16_t colorFor(DetectionType) { return CYAN; }

// EN-only: the pair returns the EN string (Settings::lang() is 0).
inline const char* tr(const char* en, const char* /*ru*/) { return en; }

// Cyrillic? True only when the string actually holds any — the ASCII pools
// are what this port draws.
inline bool isRu(const char* s) {
  if (!s) return false;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
    if (*p >= 0xD0 && *(p + 1)) return true;
  return false;
}

// The bubble face: the hg 5x7 font at the size the caller set. The original
// rungs (GLCD at 1, font2 at 1..3) map onto text sizes 1..3 directly.
inline void speechFontOn(TFT_eSPI& t, bool /*small*/) { t.setTextSize(1); }
inline void speechFontSmaller(TFT_eSPI& t) { if (t.textWidthUnit() > 1) t.setTextSize(t.textWidthUnit() - 1); }
inline bool speechAboveNormal() { return false; }
inline int  speechScale()  { return 1; }
inline int  speechTextH()  { return 8; }
inline int  speechAscent() { return 0; }
inline void bubbleFontOn(TFT_eSPI& t)  { t.setTextSize(1); }
inline void bubbleFontOff(TFT_eSPI& t) { t.setTextSize(1); }

// RU text: the hg font has Cyrillic glyphs, so the mixed-face functions can
// just delegate (they are only ever called with `isRu()` strings in v1).
inline void printRU(TFT_eSPI& t, const char* s) { t.print(s); }
inline int  textWidthRU(TFT_eSPI& t, const char* s) { return t.textWidth(s); }
inline void printRU16(TFT_eSPI& t, const char* s) { t.print(s); }
inline int  textWidthRU16(TFT_eSPI& t, const char* s) { return t.textWidth(s); }

// Word wrap to pixel width `maxW`, into fixed 48/80-char rows. Greedy on
// words; a word longer than the row is cut at the boundary.
template <int ROW>
uint8_t wrap_chars(TFT_eSPI& t, const char* text, int maxW, char lines[][ROW], uint8_t maxLines) {
  if (!text || maxLines == 0) return 0;
  uint8_t count = 0;
  const char* p = text;
  while (*p && count < maxLines) {
    // One line: extend token by token while it fits maxW. The first token of
    // a line is always committed, so the loop always makes progress.
    int w = 0;              // pixels committed to the current line
    int len = 0;            // bytes committed
    const char* q = p;
    while (*q && *q != '\n') {
      const char* tok = q;
      if (*q == ' ') { ++q; }
      else { while (*q && *q != ' ' && *q != '\n') ++q; }
      int tw = t.textWidth(std::string(tok, q - tok).c_str());
      int gap = len ? t.textWidth(" ") : 0;
      if (len && (w + gap + tw > maxW || len + (q - tok) >= ROW - 1)) break;
      w += gap + tw;
      std::memcpy(lines[count] + len, tok, q - tok);
      len += static_cast<int>(q - tok);
      lines[count][len] = 0;
      if (w > maxW) break;  // a monster token: cut it at the row width
    }
    ++count;
    p = (*q == '\n') ? q + 1 : q;
    if (*p == ' ') ++p;
  }
  return count;
}

inline uint8_t wrapText(TFT_eSPI& t, const char* text, int maxW, char lines[][48], uint8_t maxLines) {
  return wrap_chars<48>(t, text, maxW, lines, maxLines);
}
inline uint8_t wrapText(TFT_eSPI& t, const char* text, int maxW, char lines[][80], uint8_t maxLines) {
  return wrap_chars<80>(t, text, maxW, lines, maxLines);
}
inline uint8_t wrapTextRU(TFT_eSPI& t, const char* text, int maxW, char lines[][48], uint8_t maxLines) {
  return wrap_chars<48>(t, text, maxW, lines, maxLines);
}
inline uint8_t wrapTextRU(TFT_eSPI& t, const char* text, int maxW, char lines[][80], uint8_t maxLines) {
  return wrap_chars<80>(t, text, maxW, lines, maxLines);
}

// ---- stubs (see docs/potsy-port-spec.md §стабы) ------------------------

inline void showToast(const char*, const char*, uint16_t, uint32_t = 1500) {}

// The CHROME WING flock wing: v1 draws the plain wing disc instead.
inline void drawWing(TFT_eSPI& t, float sx, float sy, float len, float angDeg,
                     float flap, float width, uint8_t,
                     uint16_t body, uint16_t, float, float, uint16_t = 0xD6DB) {
  float a = angDeg * 3.14159265f / 180.0f;
  float fx = sx + std::cos(a) * len;
  float fy = sy + std::sin(a) * len + flap;
  t.drawWideLine(sx, sy, fx, fy, width, body);
  t.fillCircle(static_cast<int>(fx), static_cast<int>(fy), static_cast<int>(width / 2), body);
}

// Background easter-egg hooks: no backgrounds in this port, nothing pending.
inline bool    consumeEyeCatch()      { return false; }
inline bool    consumeLodgeKnock()    { return false; }
inline bool    consumeOwlReek()       { return false; }
inline bool    consumeToasterCatch()  { return false; }
inline bool    consumeToasterPetCatch() { return false; }
inline bool    consumeSharkCatch()    { return false; }
inline bool    consumeRedGlyph()      { return false; }
inline uint8_t consumeXyzzy()         { return 0; }
inline void    setRedGlyph(bool)      {}

}  // namespace Theme
