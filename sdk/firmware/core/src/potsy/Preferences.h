// Potsy port compat: Preferences (NVS) stub. RAM-backed, never persisted —
// Potsy's unlocks/cosmetics reset on reboot here, which v1 accepts.
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

class Preferences {
 public:
  bool begin(const char*, bool = false) { return true; }
  void end() {}

  bool getBool(const char* key, bool def) {
    Entry* e = find(key);
    return e && e->kind == 'b' ? e->b : def;
  }
  uint8_t getUChar(const char* key, uint8_t def) {
    Entry* e = find(key);
    return e && e->kind == 'u' ? e->u : def;
  }
  uint32_t getUInt(const char* key, uint32_t def) {
    Entry* e = find(key);
    return e && e->kind == 'U' ? e->U : def;
  }
  size_t getString(const char* key, char* value, size_t maxLen) {
    Entry* e = find(key);
    const char* s = e && e->kind == 's' ? e->s.c_str() : "";
    size_t n = std::min(std::strlen(s), maxLen ? maxLen - 1 : 0);
    std::memcpy(value, s, n);
    value[n] = 0;
    return n;
  }
  std::string getString(const char* key, const char* def) {
    Entry* e = find(key);
    return e && e->kind == 's' ? e->s : std::string(def);
  }

  bool putBool(const char* key, bool v) { Entry* e = set(key); e->kind = 'b'; e->b = v; return true; }
  bool putUChar(const char* key, uint8_t v) { Entry* e = set(key); e->kind = 'u'; e->u = v; return true; }
  bool putUInt(const char* key, uint32_t v) { Entry* e = set(key); e->kind = 'U'; e->U = v; return true; }
  bool putString(const char* key, const char* v) { Entry* e = set(key); e->kind = 's'; e->s = v; return true; }

  bool remove(const char* key) {
    for (size_t i = 0; i < kMax; ++i) {
      if (used_[i] && std::string(names_[i]) == key) { used_[i] = false; return true; }
    }
    return false;
  }
  void clear() { for (size_t i = 0; i < kMax; ++i) used_[i] = false; }

 private:
  static constexpr size_t kMax = 48;
  struct Entry {
    char kind = 0;
    bool b = false;
    uint8_t u = 0;
    uint32_t U = 0;
    std::string s;
  };
  char names_[kMax][24] = {};
  bool used_[kMax] = {};
  Entry entries_[kMax];

  Entry* find(const char* key) {
    for (size_t i = 0; i < kMax; ++i)
      if (used_[i] && std::string(names_[i]) == key) return &entries_[i];
    return nullptr;
  }
  Entry* set(const char* key) {
    Entry* e = find(key);
    if (e) return e;
    for (size_t i = 0; i < kMax; ++i) {
      if (!used_[i]) {
        used_[i] = true;
        std::snprintf(names_[i], sizeof(names_[i]), "%s", key);
        return &entries_[i];
      }
    }
    return &entries_[0];
  }
};
