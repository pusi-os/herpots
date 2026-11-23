// Potsy port compat: the Arduino surface potsy.cpp actually touches.
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <ctime>

#ifdef ESP_PLATFORM
#include <esp_timer.h>
inline uint32_t potsy_millis() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
}
inline uint32_t potsy_micros() {
  return static_cast<uint32_t>(esp_timer_get_time());
}
#else
#include <chrono>
inline uint32_t potsy_millis() {
  using namespace std::chrono;
  return static_cast<uint32_t>(
      duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
inline uint32_t potsy_micros() {
  using namespace std::chrono;
  return static_cast<uint32_t>(
      duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}
#endif

// Arduino random(a, b): a xorshift PRNG, seeded once from the wall clock.
inline long potsy_random(long howbig) {
  static uint32_t s = 0;
  if (!s) s = static_cast<uint32_t>(potsy_micros()) | 1u;
  s ^= s << 13;
  s ^= s >> 17;
  s ^= s << 5;
  if (howbig <= 0) return 0;
  return static_cast<long>(s % static_cast<uint32_t>(howbig));
}
inline long potsy_random_range(long howsmall, long howbig) {
  if (howsmall >= howbig) return howsmall;
  return howsmall + potsy_random(howbig - howsmall);
}

// Arduino's min/max are macros; the real ones from <algorithm> are what the
// shim and potsy.cpp end up using (C++ headers, not the Arduino macros).
#include <algorithm>
using std::min;
using std::max;
using std::abs;

// The Arduino global API potsy.cpp touches.
inline uint32_t millis() { return potsy_millis(); }
inline uint32_t micros() { return potsy_micros(); }
inline long random(long howbig) { return potsy_random(howbig); }
inline long random(long howsmall, long howbig) { return potsy_random_range(howsmall, howbig); }
