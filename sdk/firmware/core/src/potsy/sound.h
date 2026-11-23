// Potsy port compat: the speaker. The hercharm firmware owns its own audio;
// Potsy's sounds are stubbed to no-ops (see docs/potsy-port-spec.md).
#pragma once
#include <stdint.h>

namespace Sound {
    enum class Id : uint8_t {
        NEW_DEVICE = 0,
        WATCH_HIT,
        LOW_BATTERY,
        MESSAGE,
        THROW,
        LAND,
        COUNT
    };
    inline void play(Id) {}
}  // namespace Sound
