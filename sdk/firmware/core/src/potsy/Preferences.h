// Potsy port compat: Preferences (NVS) API over the port's hg::Storage.
// PotsCharm's own cosmetics namespace (outfits, shades, shirts, glasses,
// the Gods Mode flag) persists across reboots now — the values live in the
// same store the app's settings use, keys are short enough for NVS and do
// not collide with the hgadget setting keys.
//
// The storage is registered once at boot: potsy_glue::set_storage() from
// the app (esp32 main owns the one hg::Storage instance). Reads before
// registration and failed writes keep the in-memory default, same deal as
// the RAM stub this replaces. The Arduino namespace argument is accepted
// and ignored: the port's store is already namespaced.
#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#include "hg/hal.hpp"

namespace potsy_prefs {

// One hg::Storage for every Preferences instance. Set once from the app.
inline hg::Storage* storage = nullptr;

inline void use_storage(hg::Storage* store) { storage = store; }

inline std::optional<std::string> read(const char* key) {
  return storage ? storage->get(key) : std::nullopt;
}
inline bool write(const char* key, std::string_view value) {
  return storage ? storage->set(key, value) : false;
}

}  // namespace potsy_prefs

class Preferences {
 public:
  bool begin(const char*, bool = false) { return true; }  // namespace kept for source compat
  void end() {}

  bool getBool(const char* key, bool def) {
    auto v = potsy_prefs::read(key);
    if (!v || v->empty()) return def;
    return (*v)[0] != '0';
  }
  uint8_t getUChar(const char* key, uint8_t def) {
    auto v = potsy_prefs::read(key);
    if (!v || v->empty()) return def;
    return static_cast<uint8_t>(std::atoi(v->c_str()));
  }
  uint32_t getUInt(const char* key, uint32_t def) {
    auto v = potsy_prefs::read(key);
    if (!v || v->empty()) return def;
    return static_cast<uint32_t>(std::strtoul(v->c_str(), nullptr, 10));
  }
  size_t getString(const char* key, char* value, size_t maxLen) {
    auto v = potsy_prefs::read(key);
    const char* s = v ? v->c_str() : "";
    size_t n = std::min(std::strlen(s), maxLen ? maxLen - 1 : 0);
    std::memcpy(value, s, n);
    value[n] = 0;
    return n;
  }
  std::string getString(const char* key, const char* def) {
    auto v = potsy_prefs::read(key);
    return v ? *v : std::string(def);
  }

  bool putBool(const char* key, bool v) { return potsy_prefs::write(key, v ? "1" : "0"); }
  bool putUChar(const char* key, uint8_t v) { return potsy_prefs::write(key, std::to_string(v)); }
  bool putUInt(const char* key, uint32_t v) { return potsy_prefs::write(key, std::to_string(v)); }
  bool putString(const char* key, const char* v) { return potsy_prefs::write(key, v); }

  bool remove(const char* key) {
    if (!potsy_prefs::storage) return false;
    potsy_prefs::storage->erase(key);
    return true;
  }
  void clear() {}  // the store outlives one namespace; the stub's bulk wipe has no owner here
};
