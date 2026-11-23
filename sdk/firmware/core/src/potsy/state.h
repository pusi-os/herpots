// Potsy port compat: the slice of PotsCharm include/state.h potsy.cpp needs.
// DetectionType and Confidence copied verbatim from the source repo.
#pragma once
#include <stdint.h>

enum class DetectionType : uint8_t {
    UNKNOWN = 0,
    FLOCK   = 1,   // Flock Safety camera / sensor
    AXON    = 2,   // Axon body camera / LE equipment
    META    = 3,   // Ray-Ban Meta smart glasses
    SKIMMER = 4,   // HC-05/06/03 Bluetooth skimmer
    MESH    = 5,   // Meshtastic / MeshCore / RNode mesh node
    AIRTAG  = 6,   // Apple AirTag / FindMy
    DRONE   = 7,   // OpenDroneID drone
    ALPR    = 8,   // Motorola / Vigilant ALPR
    CAMERA  = 9,   // Generic camera (existing OUI list)
    SAMSUNG_TAG = 10,  // Samsung Galaxy SmartTag / SmartTag+
    GOOGLE_TAG  = 11,  // Google Find My Device network tracker
    TILE    = 12,  // Tile BLE tracker
    RING    = 13,  // Ring doorbell/camera
    DEAUTH  = 14,  // WiFi deauth/disassoc flood
    EVILTWIN = 15, // rogue/spoofed AP
    IBEACON = 16,  // Apple iBeacon proximity beacon
    HACKER  = 17,  // pentest hardware (exact signatures only)
    COUNT   = 18
};

inline const char* detectionTypeName(DetectionType t) {
    switch (t) {
        case DetectionType::FLOCK:       return "FLOCK";
        case DetectionType::AXON:        return "AXON";
        case DetectionType::META:        return "META";
        case DetectionType::SKIMMER:     return "SKIMMER";
        case DetectionType::MESH:        return "MESH";
        case DetectionType::AIRTAG:      return "AIRTAG";
        case DetectionType::DRONE:       return "DRONE";
        case DetectionType::ALPR:        return "ALPR";
        case DetectionType::CAMERA:      return "CAMERA";
        case DetectionType::SAMSUNG_TAG: return "SAMSUNG_TAG";
        case DetectionType::GOOGLE_TAG:  return "GOOGLE_TAG";
        case DetectionType::TILE:        return "TILE";
        case DetectionType::RING:        return "RING";
        case DetectionType::DEAUTH:      return "DEAUTH";
        case DetectionType::EVILTWIN:    return "EVIL TWIN";
        case DetectionType::IBEACON:     return "IBEACON";
        case DetectionType::HACKER:      return "HACKER";
        default:                         return "UNKNOWN";
    }
}

// Note: plain LOW/MEDIUM/HIGH collide with the Arduino core's pin-state
// macros, hence the _CONF suffix.
enum class Confidence : uint8_t { LOW_CONF, MED_CONF, HIGH_CONF };
