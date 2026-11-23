// Potsy port compat: the POTSY-DEX. Only the rarity lookup potsy.cpp uses
// survives the port; every type reads COMMON.
#pragma once
#include <stdint.h>
#include "state.h"

namespace Dex {

enum class Rarity : uint8_t { COMMON, UNCOMMON, RARE };
inline Rarity rarity(DetectionType) { return Rarity::COMMON; }

}  // namespace Dex
