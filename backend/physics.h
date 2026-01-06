#pragma once

#include <cstdint>
#include <vector>

#include "config.h"
#include "protocol.h"

struct Particle {
  float x;
  float y;
  float vx;
  float vy;
};

struct Projectile {
  float x;
  float y;
  float vx;
  float vy;
  float life;
  uint32_t type;
};

class Simulation {
 public:
  Simulation(int num_particles, float width, float height);

  void Step(const protocol::InputFrame& input);

  const std::vector<float>& RenderBuffer() const { return render_buffer_; }
  int NumParticles() const { return num_particles_; }

 private:
  void SpawnProjectile(const protocol::InputFrame& input);
  void UpdateProjectiles();
  void InitialiseParticle(int index, uint32_t& seed);
  void ResetParticle(int index, uint32_t& seed);
  void SetupRoute(int index, uint32_t& seed, float center_x, float center_y);

  int num_particles_ = 0;
  float width_ = 0.0f;
  float height_ = 0.0f;
  uint32_t seed_ = 0x12345678u;
  uint32_t frame_ = 0u;

  std::vector<Particle> particles_;
  std::vector<float> respawn_timers_;
  std::vector<float> route_center_x_;
  std::vector<float> route_center_y_;
  std::vector<float> route_radius_;
  std::vector<float> route_phase_;
  std::vector<float> route_omega_;
  std::vector<Projectile> projectiles_;
  std::vector<Projectile> projectiles_next_;
  std::vector<float> render_buffer_;
};
