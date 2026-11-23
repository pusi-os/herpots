// Potsy port compat: a minimal TFT_eSPI-shaped shim over hg::Canvas.
// Only the primitives potsy.cpp actually uses are implemented (see
// docs/potsy-port-spec.md for the measured call frequencies). Coordinates
// are full-canvas: the hg::Canvas band/clip machinery drops what falls
// outside the rows currently being rendered, so the shim just delegates.
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>

#include "hg/canvas.hpp"
#include "hg/font.hpp"

#include <cmath>
// The TFT_eSPI colour macros potsy.cpp uses.
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF

// millis()/micros()/random() live in the Arduino.h stub (potsy.cpp includes
// it directly); declared here too because the shim's text path is shared.

class TFT_eSPI {
 public:
  explicit TFT_eSPI(hg::Canvas* c) : c_(c) {}

  int16_t width() const { return static_cast<int16_t>(c_->width()); }
  int16_t height() const { return static_cast<int16_t>(c_->height()); }

  // ---- primitives ----
  void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    c_->fill_rect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h),
                  static_cast<uint16_t>(color));
  }
  void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    c_->rect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h),
             static_cast<uint16_t>(color));
  }
  void fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color) {
    c_->fill_round_rect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h),
                        static_cast<int>(r), static_cast<uint16_t>(color));
  }
  void drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color) {
    // 1px outline as four fills (Canvas::rect rounds corners square, matching
    // the GLCD look better than a ring).
    int x0 = static_cast<int>(x), y0 = static_cast<int>(y);
    int ww = static_cast<int>(w), hh = static_cast<int>(h), rr = static_cast<int>(r);
    uint16_t col = static_cast<uint16_t>(color);
    c_->fill_rect(x0 + rr, y0, ww - 2 * rr, 1, col);
    c_->fill_rect(x0 + rr, y0 + hh - 1, ww - 2 * rr, 1, col);
    c_->fill_rect(x0, y0 + rr, 1, hh - 2 * rr, col);
    c_->fill_rect(x0 + ww - 1, y0 + rr, 1, hh - 2 * rr, col);
  }
  void fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
    // Standard barycentric scan over the bounding box (TFT_eSPI's flat-bottom
    // optimisation is not worth reproducing; these are small triangles).
    using L = long long;
    L ax = x0, ay = y0, bx = x1, by = y1, cx = x2, cy = y2;
    L det = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay);
    if (det == 0) return;
    uint16_t col = static_cast<uint16_t>(color);
    L miny = std::min({ay, by, cy}), maxy = std::max({ay, by, cy});
    L minx = std::min({ax, bx, cx}), maxx = std::max({ax, bx, cx});
    for (L y = miny; y <= maxy; ++y) {
      bool have = false;
      L left = 0, right = 0;
      for (L x = minx; x <= maxx; ++x) {
        L w0 = ((bx - ax) * (y - ay) - (x - ax) * (by - ay)) * ((cx - ax) * (y - ay) - (x - ax) * (cy - ay));
        L w1 = ((cx - bx) * (y - by) - (x - bx) * (cy - by)) * ((ax - bx) * (y - by) - (x - bx) * (ay - by));
        L w2 = ((ax - cx) * (y - cy) - (x - cx) * (ay - cy)) * ((bx - cx) * (y - cy) - (x - cx) * (by - cy));
        bool inside = (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
        if (inside) {
          if (!have) { left = x; right = x; have = true; }
          else right = x;
        }
      }
      if (have) c_->fill_rect(static_cast<int>(left), static_cast<int>(y),
                              static_cast<int>(right - left + 1), 1, col);
    }
  }
  void fillCircle(int32_t cx, int32_t cy, int32_t r, uint32_t color) {
    c_->fill_circle(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r),
                    static_cast<uint16_t>(color));
  }
  void drawCircle(int32_t cx, int32_t cy, int32_t r, uint32_t color) {
    c_->ring(static_cast<int>(cx), static_cast<int>(cy), static_cast<int>(r), 1,
             static_cast<uint16_t>(color));
  }
  void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    // Bresenham, pixel by pixel.
    int dx = abs(static_cast<int>(x1 - x0)), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(static_cast<int>(y1 - y0)), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    uint16_t col = static_cast<uint16_t>(color);
    for (;;) {
      c_->pixel(static_cast<int>(x0), static_cast<int>(y0), col);
      if (x0 == x1 && y0 == y1) break;
      int e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
    }
  }
  void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color) {
    c_->fill_rect(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), 1,
                  static_cast<uint16_t>(color));
  }
  void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color) {
    c_->fill_rect(static_cast<int>(x), static_cast<int>(y), 1, static_cast<int>(h),
                  static_cast<uint16_t>(color));
  }
  void drawPixel(int32_t x, int32_t y, uint32_t color) {
    c_->pixel(static_cast<int>(x), static_cast<int>(y), static_cast<uint16_t>(color));
  }
  uint32_t readPixel(int32_t x, int32_t y) const {
    return c_->read_pixel(static_cast<int>(x), static_cast<int>(y));
  }
  // Ellipses: X^2/rx^2 + Y^2/ry^2 <= 1, filled row spans / 1px outline.
  void fillEllipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, uint32_t color) {
    if (rx <= 0 || ry <= 0) return;
    uint16_t col = static_cast<uint16_t>(color);
    for (int dy = -static_cast<int>(ry); dy <= static_cast<int>(ry); ++dy) {
      long long num = static_cast<long long>(rx) * rx *
                      static_cast<long long>(ry) * ry - static_cast<long long>(dy) * dy * rx * rx;
      long long dx = num / (static_cast<long long>(ry) * ry);
      // sqrt via doubling: dx2 = floor(sqrt(num / ry^2)) in pixel terms
      long long span = 0;
      while ((span + 1) * (span + 1) * static_cast<long long>(ry) * ry <= num) ++span;
      (void)dx;
      c_->fill_rect(static_cast<int>(cx) - static_cast<int>(span), static_cast<int>(cy) + dy,
                    static_cast<int>(2 * span + 1), 1, col);
    }
  }
  void drawEllipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, uint32_t color) {
    if (rx <= 0 || ry <= 0) return;
    uint16_t col = static_cast<uint16_t>(color);
    for (int dy = -static_cast<int>(ry); dy <= static_cast<int>(ry); ++dy) {
      long long num = static_cast<long long>(rx) * rx * static_cast<long long>(ry) * ry -
                      static_cast<long long>(dy) * dy * rx * rx;
      long long span = 0;
      while ((span + 1) * (span + 1) * static_cast<long long>(ry) * ry <= num) ++span;
      c_->pixel(static_cast<int>(cx) - static_cast<int>(span), static_cast<int>(cy) + dy, col);
      c_->pixel(static_cast<int>(cx) + static_cast<int>(span), static_cast<int>(cy) + dy, col);
    }
  }
  // Capsule: two end circles + the quad between them (integer, no blending —
  // the same shape potsy.cpp's own wideLine() helper draws).
  void drawWideLine(float ax, float ay, float bx, float by, float wd, uint32_t color) {
    uint16_t col = static_cast<uint16_t>(color);
    float dx = bx - ax, dy = by - ay;
    float len = std::sqrt(dx * dx + dy * dy);
    int r = static_cast<int>(wd / 2);
    if (len < 0.01f) { fillCircle(static_cast<int>(ax), static_cast<int>(ay), r, col); return; }
    float nx = -dy / len * (wd / 2), ny = dx / len * (wd / 2);
    fillTriangle(static_cast<int>(ax + nx), static_cast<int>(ay + ny),
                 static_cast<int>(ax - nx), static_cast<int>(ay - ny),
                 static_cast<int>(bx + nx), static_cast<int>(by + ny), col);
    fillTriangle(static_cast<int>(bx + nx), static_cast<int>(by + ny),
                 static_cast<int>(bx - nx), static_cast<int>(by - ny),
                 static_cast<int>(ax - nx), static_cast<int>(ay - ny), col);
    fillCircle(static_cast<int>(ax), static_cast<int>(ay), r, col);
    fillCircle(static_cast<int>(bx), static_cast<int>(by), r, col);
  }
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) { return hg::rgb565(r, g, b); }
  uint8_t color16to8(uint16_t c) {
    // RGB565 -> RGB332, the TFT_eSPI convention.
    return static_cast<uint8_t>(((c >> 8) & 0xE0) | ((c >> 6) & 0x1C) | ((c >> 3) & 0x07));
  }

  // ---- text (the hg 5x7 font, via Canvas::text) ----
  void setTextSize(uint8_t s) { textsize_ = s < 1 ? 1 : s; }
  uint8_t textWidthUnit() const { return textsize_; }
  void setTextDatum(int) {}  // TL datum is what everything here uses
  void setTextColor(uint16_t fg) { textfg_ = fg; textbg_ = 0; }
  void setTextColor(uint16_t fg, uint16_t bg) { textfg_ = fg; textbg_ = bg; }
  void setTextWrap(bool wrap) { wrap_ = wrap; }
  void setCursor(int16_t x, int16_t y) { cursor_x_ = x; cursor_y_ = y; }
  int16_t getCursorX() const { return cursor_x_; }
  int16_t getCursorY() const { return cursor_y_; }
  int32_t textWidth(const char* s) {
    if (!s) return 0;
    return hg::Canvas::text_width(s, textsize_);
  }
  size_t print(const char* s) { return draw_text(s); }
  size_t print(const std::string& s) { return draw_text(s.c_str()); }
  size_t print(char ch) { char b[2] = {ch, 0}; return draw_text(b); }
  size_t println(const char* s) { size_t n = draw_text(s); cursor_y_ += hg::Canvas::line_height(textsize_); return n + 1; }

 private:
  size_t draw_text(const char* s) {
    if (!s) return 0;
    size_t n = std::strlen(s);
    int x = cursor_x_, y = cursor_y_;
    int cellw = 6 * textsize_;
    int cellh = hg::Canvas::line_height(textsize_);
    size_t i = 0, start = 0;
    while (i < n) {
      const char32_t cp = hg::font::next(s, i);  // advances i past the code point
      if (cp == '\n') {
        if (wrap_) { x = 0; y += cellh; }
        start = i;
        continue;
      }
      if (cp == '\r') { start = i; continue; }
      // Wrap at the right edge like the GLCD path does.
      if (wrap_ && x + cellw > c_->width()) { x = 0; y += cellh; }
      c_->text(x, y, std::string_view(s + start, i - start), textsize_, textfg_);
      x += hg::font::columns(cp) * cellw;
      start = i;
    }
    cursor_x_ = static_cast<int16_t>(x);
    cursor_y_ = static_cast<int16_t>(y);
    return n;
  }

  hg::Canvas* c_;
  uint8_t textsize_ = 1;
  int16_t cursor_x_ = 0, cursor_y_ = 0;
  uint16_t textfg_ = 0xFFFF, textbg_ = 0;
  bool wrap_ = true;
};
