// Potsy port compat: the Settings surface potsy.cpp (and its potsy_ru.inc)
// uses. v1 is EN-only: lang() returns 0, so every RU pool compiles but is
// never picked. Everything here is constexpr on purpose — the compiler then
// folds isRU() to false, dead-code-eliminates the RU text pools and the
// background easter-egg paths, and the vendored potsy.cpp stays byte-identical.
//
// The background is the one runtime knob (owner request): the persona message
// from Hermes and the settings menu set it, and it persists in Potsy's own
// Preferences namespace (key "bg", the PotsCharm key). Everything else stays
// compile-time. The storage behind Preferences is the port's hg::Storage once
// potsy_glue::set_storage() runs; reads before that yield the default.
#pragma once
#include <stdint.h>

#include "Preferences.h"

namespace Settings {

    enum class Background : uint8_t {
        DIGITAL = 0, STARFIELD = 1, TOASTERS = 2,
        FIREFLIES = 3, BLACK = 4
    };
    static constexpr uint8_t BACKGROUND_COUNT = 5;

    // DIGITAL is the DIGITAL-rain renderer (potsy glue's rain.cpp); the other
    // values are their own animated scenes (the paint_* renderers in
    // glue.cpp), all pure functions of the frame clock. BLACK stays black.
    inline uint8_t  s_background = static_cast<uint8_t>(Background::DIGITAL);
    inline bool     s_backgroundLoaded = false;

    inline Background background() {
        if (!s_backgroundLoaded) {
            Preferences prefs;
            prefs.begin("settings", false);
            s_background = prefs.getUChar("bg", static_cast<uint8_t>(Background::DIGITAL));
            if (s_background >= BACKGROUND_COUNT) s_background = 0;
            s_backgroundLoaded = true;
        }
        return static_cast<Background>(s_background);
    }

    inline void setBackground(uint8_t value) {
        if (value >= BACKGROUND_COUNT) value = static_cast<uint8_t>(Background::DIGITAL);
        s_background = value;
        s_backgroundLoaded = true;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putUChar("bg", value);
    }

    // The persona knobs are runtime now (they came from Hermes' persona message
    // or the settings menu) and persist in Potsy's Preferences namespace next to
    // the background. They keep a default until something sets one, so a device
    // that never heard from Hermes behaves exactly as the compile-time build did.
    // Storage behind Preferences is the port's hg::Storage once
    // potsy_glue::set_storage() runs; reads before that yield the default.
    inline uint8_t s_banter = 2;      // 0 quiet, 1 normal, 2 talkative
    inline uint8_t s_tempoPct = 100;  // his animation speed, percent of the designed one
    inline bool    s_mini = true;     // whether he also stands on the clock face
    inline bool    s_knobsLoaded = false;

    inline void loadKnobs() {
        if (s_knobsLoaded) return;
        Preferences prefs;
        prefs.begin("settings", false);
        s_banter = prefs.getUChar("banter", 2);
        if (s_banter > 2) s_banter = 2;
        s_tempoPct = prefs.getUChar("tempo", 100);
        if (s_tempoPct < 50) s_tempoPct = 50;
        if (s_tempoPct > 200) s_tempoPct = 200;
        s_mini = prefs.getBool("mini", true);
        s_knobsLoaded = true;
    }

    constexpr bool       auraShown()      { return false; }
    // 1 = RU: the RU banks in potsy_ru.inc are what the device speaks. The
    // pools stay in flash either way; this only picks which one is read.
    constexpr uint8_t    lang()           { return 1; }  // 0 EN, 1 RU
    // Idle chatter and the tempo are read from the runtime knobs above; both
    // used to be constexpr here, which is why potsy.cpp asks for them per frame.
    inline uint8_t       banter()         { loadKnobs(); return s_banter; }
    inline float         banterScale()    { return banter() == 0 ? 2.5f : banter() == 1 ? 1.0f : 0.5f; }
    inline uint8_t       mascotTempoPct() { loadKnobs(); return s_tempoPct; }
    // Whether the clock face draws the mini Potsy in its avatar slot.
    inline bool          miniOnClock()    { loadKnobs(); return s_mini; }

    // Setters: the persona message and the settings menu. Each persists, so the
    // device keeps the look after a reboot or while it is offline.
    inline void setBanter(uint8_t value) {
        if (value > 2) value = 2;
        s_banter = value;
        s_knobsLoaded = true;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putUChar("banter", value);
    }
    inline uint8_t currentTempo() { loadKnobs(); return s_tempoPct; }
    inline void setTempoPct(uint8_t value) {
        if (value < 50) value = 50;
        if (value > 200) value = 200;
        s_tempoPct = value;
        s_knobsLoaded = true;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putUChar("tempo", value);
    }
    inline void setMini(bool value) {
        s_mini = value;
        s_knobsLoaded = true;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("mini", value);
    }

}  // namespace Settings
