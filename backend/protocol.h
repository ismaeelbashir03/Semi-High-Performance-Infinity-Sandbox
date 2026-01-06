#pragma once

#include <cstdint>

namespace protocol {

enum Technique : uint32_t {
  kTechBlue = 1,
  kTechRed = 2,
  kTechPurple = 3,
};

#pragma pack(push, 1) // pack structs so no padding in between
struct InitMsg {
  uint32_t magic;
  uint32_t width;
  uint32_t height;
  uint32_t num_particles;
};

struct InputFrame {
  uint32_t technique;
  float player_x;
  float player_y;
  uint32_t fire;
  float dir_x;
  float dir_y;
  uint32_t barrier_on;
  float inf_radius;
  float inf_power;
  float inf_jitter;
  float inf_push;
  float projectile_speed;
  float projectile_field;
};
#pragma pack(pop)

static_assert(sizeof(InitMsg) == 16, "InitMsg must be packed");
static_assert(sizeof(InputFrame) == 52, "InputFrame must be packed");

}
