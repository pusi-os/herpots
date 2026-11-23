// Potsy port compat: wall-clock stub on top of the C library. The hercharm
// port fills the system time from its own wall-time machinery; everything
// degrades to uptime when none is set, exactly as the original does.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <cstdio>
#include <algorithm>

namespace Clock {

inline uint32_t s_greetedDay_ = 0;
inline uint32_t s_milestone_ = 0;
inline uint32_t s_born_ = 0;

inline bool isSet() {
  time_t now = time(nullptr);
  return now > 1735689600;  // anything after 2025-01-01 counts as a real clock
}
inline bool trusted() { return isSet(); }
inline bool guessed() { return false; }

inline uint8_t hour() {
  if (!isSet()) return 0;
  time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  return static_cast<uint8_t>(lt.tm_hour);
}
inline uint8_t weekday() {
  if (!isSet()) return 0;
  time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  return static_cast<uint8_t>(lt.tm_wday);  // Sunday first, as the original
}
inline uint32_t localDay() {
  if (!isSet()) return 0;
  time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  lt.tm_hour = lt.tm_min = lt.tm_sec = 0;
  return static_cast<uint32_t>(mktime(&lt) / 86400);
}
inline bool weekend() { uint8_t d = weekday(); return d == 0 || d == 6; }
inline bool night() { uint8_t h = hour(); return h >= 23 || h < 5; }

// "SUN SEP 14" -- the desk's date line.
inline void formatDate(char* out, size_t n) {
  static const char* WD[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  static const char* MO[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                             "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  if (!isSet()) { std::snprintf(out, n, "not set"); return; }
  time_t t = time(nullptr);
  struct tm lt;
  localtime_r(&t, &lt);
  std::snprintf(out, n, "%s %s %d", WD[std::min(6, std::max(0, lt.tm_wday - 1))],
                MO[std::min(11, std::max(0, lt.tm_mon))], lt.tm_mday);
}

inline uint32_t greetedDay()                  { return s_greetedDay_; }
inline void     setGreetedDay(uint32_t day)   { s_greetedDay_ = day; }
inline uint32_t milestoneSaid()               { return s_milestone_; }
inline void     setMilestoneSaid(uint32_t d)  { s_milestone_ = d; }

// daysTogether(): days since the clock first learned a date. No persistence
// in the stub: it counts from the first set day of this boot.
inline uint32_t daysTogether() {
  if (!isSet()) return 0;
  if (!s_born_) s_born_ = localDay();
  return localDay() - s_born_;
}

}  // namespace Clock
