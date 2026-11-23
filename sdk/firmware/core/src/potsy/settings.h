// Potsy port compat: the Settings surface potsy.cpp (and its potsy_ru.inc)
// uses. v1 is EN-only: lang() returns 0, so every RU pool compiles but is
// never picked. Everything here is constexpr on purpose — the compiler then
// folds isRU() to false, dead-code-eliminates the RU text pools and the
// background easter-egg paths, and the vendored potsy.cpp stays byte-identical.
#pragma once
#include <stdint.h>

namespace Settings {

    enum class Background : uint8_t {
        DIGITAL = 0, STARFIELD = 1, TOASTERS = 2,
        AQUARIUM = 3, TERMINAL = 4, FIREFLIES = 5,
        FIRE = 6, SNOWFALL = 7, SPECTRUM = 8,
        TUNNEL = 9, SYNTHWAVE = 10, BLACK = 11
    };
    static constexpr uint8_t BACKGROUND_COUNT = 12;

    constexpr Background background()     { return Background::DIGITAL; }
    constexpr bool       auraShown()      { return false; }
    // 1 = RU: the RU banks in potsy_ru.inc are what the device speaks. The
    // pools stay in flash either way; this only picks which one is read.
    constexpr uint8_t    lang()           { return 1; }  // 0 EN, 1 RU
    constexpr uint8_t    banter()         { return 2; }  // nonzero: idle chatter stays on
    constexpr float      banterScale()    { return 1.0f; }
    constexpr uint8_t    mascotTempoPct() { return 100; }

}  // namespace Settings
