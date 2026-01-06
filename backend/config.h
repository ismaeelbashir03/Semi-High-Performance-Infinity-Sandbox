#pragma once

#include <cstdint>

namespace config {
constexpr uint32_t kInitMagic = 0x31544D4C; // LMT1 in little endian 
constexpr int kDefaultPort = 9999;
constexpr int kMaxProjectiles = 8;

constexpr float kDt = 1.0f / 60.0f;
constexpr float kEps = 1e-3f;
constexpr float kGlobalDamp = 0.995f;
constexpr float kMaxSpeed = 2500.0f;

constexpr float kProjectileSpeedBlue = 1100.0f;
constexpr float kProjectileSpeedRed = 1150.0f;
constexpr float kProjectileSpeedPurple = 1250.0f;

constexpr float kProjectileRadiusBlue = 90.0f;
constexpr float kProjectileRadiusRed = 90.0f;
constexpr float kProjectileRadiusPurple = 70.0f;

constexpr float kProjectileForceBlue = 5200.0f;
constexpr float kProjectileForceRed = 5600.0f;

constexpr float kProjectileLifeBlue = 5.0f;
constexpr float kProjectileLifeRed = 5.0f;
constexpr float kProjectileLifePurple = 5.0f;

constexpr float kProjectileSpeedScaleMin = 0.2f;
constexpr float kProjectileSpeedScaleMax = 2.0f;
constexpr float kProjectileFieldMin = 0.1f;
constexpr float kProjectileFieldMax = 2.0f;

constexpr float kInitSpeed = 40.0f;
constexpr float kRespawnSpeed = 40.0f;

constexpr float kRouteRadiusMin = 40.0f;
constexpr float kRouteRadiusMax = 160.0f;
constexpr float kRouteOmegaMin = 0.01f;
constexpr float kRouteOmegaMax = 0.03f;
constexpr float kRoutePull = 0.08f;

constexpr float kRespawnMin = 0.25f;
constexpr float kRespawnMax = 1.2f;
constexpr float kDeadCoord = -10000.0f;
constexpr float kSpawnMargin = 20.0f;
}
