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
        AQUARIUM = 3, TERMINAL = 4, FIREFLIES = 5,
        FIRE = 6, SNOWFALL = 7, SPECTRUM = 8,
        TUNNEL = 9, SYNTHWAVE = 10, BLACK = 11
    };
    static constexpr uint8_t BACKGROUND_COUNT = 12;

    // The only drawn background in this port is the DIGITAL rain (potsy
    // glue's rain.cpp); the other values are kept for the persona protocol
    // and the easter-egg switches, and map to DIGITAL at draw time.
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

    constexpr bool       auraShown()      { return false; }
    // 1 = RU: the RU banks in potsy_ru.inc are what the device speaks. The
    // pools stay in flash either way; this only picks which one is read.
    constexpr uint8_t    lang()           { return 1; }  // 0 EN, 1 RU
    constexpr uint8_t    banter()         { return 2; }  // nonzero: idle chatter stays on
    constexpr float      banterScale()    { return 1.0f; }
    constexpr uint8_t    mascotTempoPct() { return 100; }

}  // namespace Settings
