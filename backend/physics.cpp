#include "physics.h"

#include <algorithm>
#include <cmath>

#include <omp.h>

namespace {

inline float rand01(uint32_t &state) {
  state = state * 1664525u + 1013904223u;
  return static_cast<float>(state >> 8) * (1.0f / 16777216.0f);
}

float rand_range(uint32_t &state, float min_value, float max_value) {
  return min_value + rand01(state) * (max_value - min_value);
}

/*
 * Resets a particle to a random position outside the simulation area
 * so we get like a loop around effect, but more random.
 */
void reset_particle(Particle &p, uint32_t seed, float width, float height) {
  float rx = rand01(seed);
  float ry = rand01(seed);
  uint32_t side = seed & 3u;
  if (side == 0u) {
    p.x = -config::kSpawnMargin;
    p.y = ry * height;
  } else if (side == 1u) {
    p.x = width + config::kSpawnMargin;
    p.y = ry * height;
  } else if (side == 2u) {
    p.x = rx * width;
    p.y = -config::kSpawnMargin;
  } else {
    p.x = rx * width;
    p.y = height + config::kSpawnMargin;
  }
  float vx = (rand01(seed) - 0.5f) * config::kRespawnSpeed;
  float vy = (rand01(seed) - 0.5f) * config::kRespawnSpeed;
  p.vx = vx;
  p.vy = vy;
}

float projectile_speed(uint32_t type) {
  if (type == protocol::kTechBlue) {
    return config::kProjectileSpeedBlue;
  }
  if (type == protocol::kTechRed) {
    return config::kProjectileSpeedRed;
  }
  return config::kProjectileSpeedPurple;
}

float projectile_radius(uint32_t type) {
  if (type == protocol::kTechBlue) {
    return config::kProjectileRadiusBlue;
  }
  if (type == protocol::kTechRed) {
    return config::kProjectileRadiusRed;
  }
  return config::kProjectileRadiusPurple;
}

float projectile_life(uint32_t type) {
  if (type == protocol::kTechBlue) {
    return config::kProjectileLifeBlue;
  }
  if (type == protocol::kTechRed) {
    return config::kProjectileLifeRed;
  }
  return config::kProjectileLifePurple;
}

} // namespace

Simulation::Simulation(int num_particles, float width, float height)
    : num_particles_(num_particles), width_(width), height_(height),
      particles_(num_particles), respawn_timers_(num_particles, 0.0f),
      route_center_x_(num_particles), route_center_y_(num_particles),
      route_radius_(num_particles), route_phase_(num_particles),
      route_omega_(num_particles),
      render_buffer_(static_cast<size_t>(num_particles) * 2u +
                     static_cast<size_t>(config::kMaxProjectiles) * 3u) {
  projectiles_.reserve(config::kMaxProjectiles);
  projectiles_next_.reserve(config::kMaxProjectiles);
  for (int i = 0; i < num_particles_; ++i) {
    InitialiseParticle(i, seed_);
  }
}

/*
 * Inits a particle at a random position on the screen, with a random starting
 * speed this gives a more alive feel
 */
void Simulation::InitialiseParticle(int index, uint32_t &seed) {
  Particle &p = particles_[index];
  p.x = rand01(seed) * width_;
  p.y = rand01(seed) * height_;
  p.vx = (rand01(seed) - 0.5f) * config::kInitSpeed;
  p.vy = (rand01(seed) - 0.5f) * config::kInitSpeed;
  SetupRoute(index, seed, p.x, p.y);
}

/*
 * Resets a particle to a random position
 */
void Simulation::ResetParticle(int index, uint32_t &seed) {
  Particle &p = particles_[index];
  reset_particle(p, seed, width_, height_);
  SetupRoute(index, seed, p.x, p.y);
}

/*
 * Particles should follow a route, so they have purposeful movement
 * this function sets up a circular route around a center point
 */
void Simulation::SetupRoute(int index, uint32_t &seed, float center_x,
                            float center_y) {
  float radius =
      rand_range(seed, config::kRouteRadiusMin, config::kRouteRadiusMax);
  float omega =
      rand_range(seed, config::kRouteOmegaMin, config::kRouteOmegaMax);
  if ((seed & 1u) != 0u) {
    omega = -omega;
  }
  route_center_x_[index] = center_x;
  route_center_y_[index] = center_y;
  route_radius_[index] = radius;
  route_phase_[index] = rand_range(seed, 0.0f, 6.2831853f);
  route_omega_[index] = omega;
}

/*
 * Spawns in a limiteless projectile
 */
void Simulation::SpawnProjectile(const protocol::InputFrame &input) {
  if (input.fire == 0u || projectiles_.size() >= config::kMaxProjectiles) {
    return;
  }

  uint32_t type = input.technique;
  if (type != protocol::kTechBlue && type != protocol::kTechRed &&
      type != protocol::kTechPurple) {
    return;
  }

  float len2 = input.dir_x * input.dir_x + input.dir_y * input.dir_y;
  if (len2 <= 1e-4f) {
    return;
  }

  float inv_len = 1.0f / std::sqrt(len2);
  Projectile proj{};
  proj.x = input.player_x;
  proj.y = input.player_y;
  float speed_scale =
      std::clamp(input.projectile_speed, config::kProjectileSpeedScaleMin,
                 config::kProjectileSpeedScaleMax);
  float speed = projectile_speed(type) * speed_scale;
  proj.vx = input.dir_x * inv_len * speed;
  proj.vy = input.dir_y * inv_len * speed;
  proj.life = 0.0f;
  proj.type = type;
  projectiles_.push_back(proj);
}

/*
 * Updates all projectiles, removing any that have died from purple or gone oob
 */
void Simulation::UpdateProjectiles() {
  if (projectiles_.empty()) {
    return;
  }

  projectiles_next_.clear();
  for (const auto &proj : projectiles_) {
    Projectile updated = proj;
    updated.x += updated.vx * config::kDt;
    updated.y += updated.vy * config::kDt;
    updated.life += config::kDt;
    float radius = projectile_radius(updated.type);
    float life_limit = projectile_life(updated.type);
    bool alive = updated.life < life_limit && updated.x >= -radius &&
                 updated.x <= width_ + radius && updated.y >= -radius &&
                 updated.y <= height_ + radius;
    if (alive) {
      projectiles_next_.push_back(updated);
    }
  }
  projectiles_.swap(projectiles_next_);
}

/*
 * Steps the simulation forward by one frame:
 * - Spawns projectiles based on input
 * - Updates projectiles
 * - Updates particles based on projectiles and input
 */
void Simulation::Step(const protocol::InputFrame &input) {
	SpawnProjectile(input);
	UpdateProjectiles();

	const int proj_count = static_cast<int>(projectiles_.size());
	const Projectile *proj_ptr = projectiles_.data();

	const float inf_radius = std::max(5.0f, input.inf_radius);
	const float inf_power = std::max(0.5f, input.inf_power);
	const float inf_jitter = std::max(0.0f, input.inf_jitter);
	const float inf_push = std::max(0.0f, input.inf_push);
	const bool barrier_on = input.barrier_on != 0u;
	const float field_scale = std::clamp(input.projectile_field, config::kProjectileFieldMin, config::kProjectileFieldMax);
	const float barrier_radius2 = inf_radius * inf_radius;
	const float purple_radius2 = config::kProjectileRadiusPurple * config::kProjectileRadiusPurple;

	const float player_x = input.player_x;
	const float player_y = input.player_y;

	#pragma omp parallel for schedule(static)
	for (int i = 0; i < num_particles_; ++i) {
		Particle &p = particles_[i];

		// respawn if dead and time is up
		float respawn = respawn_timers_[i];
		if (respawn > 0.0f) {
			respawn = std::max(0.0f, respawn - config::kDt);
			respawn_timers_[i] = respawn;
			if (respawn > 0.0f) {
				p.x = config::kDeadCoord;
				p.y = config::kDeadCoord;
				p.vx = 0.0f;
				p.vy = 0.0f;
				size_t out_idx = static_cast<size_t>(i) * 2u;
				render_buffer_[out_idx] = p.x;
				render_buffer_[out_idx + 1u] = p.y;
				continue;
			}
			uint32_t seed_local = static_cast<uint32_t>(i * 9781u + frame_ * 6271u);
			ResetParticle(i, seed_local);
		}


		bool destroyed = false;
		for (int s = 0; s < proj_count; ++s) {
		const Projectile &proj = proj_ptr[s];
		float dxp = proj.x - p.x;
		float dyp = proj.y - p.y;
		float dist2 = dxp * dxp + dyp * dyp + config::kEps;

		if (proj.type == protocol::kTechPurple) { // purple just merks mfs, so no force, just kill
			if (dist2 < purple_radius2) {
				uint32_t seed = static_cast<uint32_t>(i * 928371u + frame_ * 689287u + s * 131u);
				float cooldown = config::kRespawnMin + rand01(seed) * (config::kRespawnMax - config::kRespawnMin);
				respawn_timers_[i] = cooldown;
				p.x = config::kDeadCoord;
				p.y = config::kDeadCoord;
				p.vx = 0.0f;
				p.vy = 0.0f;
				destroyed = true;
				break;
			}
		} else { // others just apply force (do inverse for red etc)
			float radius = (proj.type == protocol::kTechBlue)
							? config::kProjectileRadiusBlue
							: config::kProjectileRadiusRed;
			float dist = std::sqrt(dist2);
			float inv_dist = 1.0f / dist;
			float scaled = dist / radius;
			float falloff = 1.0f / (1.0f + scaled * scaled);
			float accel = (proj.type == protocol::kTechBlue)
							? config::kProjectileForceBlue
							: config::kProjectileForceRed;
			float dir = (proj.type == protocol::kTechBlue) ? 1.0f : -1.0f;
			float strength = accel * falloff * field_scale;
			p.vx += dir * strength * dxp * inv_dist * config::kDt;
			p.vy += dir * strength * dyp * inv_dist * config::kDt;
		}
		}

		// the inf barrier should slow particles near the player accumulatively
		if (!destroyed && barrier_on) {
			float dx = p.x - player_x;
			float dy = p.y - player_y;
			float dist2 = dx * dx + dy * dy + config::kEps;
			if (dist2 < barrier_radius2) {
				float dist = std::sqrt(dist2);
				float inv_dist = 1.0f / dist;
				float nx = dx * inv_dist;
				float ny = dy * inv_dist;
				float t = dist / inf_radius;
				float damp = std::pow(t, inf_power);
				p.vx *= damp;
				p.vy *= damp;

				float radial = p.vx * nx + p.vy * ny;
				if (radial < 0.0f) {
				p.vx -= radial * nx;
				p.vy -= radial * ny;
				}
				
				// jitter effect so it looks cooler, and particles 'feel' the field more
				uint32_t jitter_seed = static_cast<uint32_t>(i) ^ (frame_ * 0x9e3779b9u);
				float jx = (rand01(jitter_seed) - 0.5f) * 2.0f;
				float jy = (rand01(jitter_seed) - 0.5f) * 2.0f;
				float jitter = (1.0f - t) * inf_jitter;
				p.x += jx * jitter;
				p.y += jy * jitter;
				
				// if we run into a particle, we are quite fast, so they will be in the barrier
				// it sucks if they slow down but are still inside, so push them out with extra force
				// but dont carry on momentum from this (makes it look more cool)
				dx = p.x - player_x;
				dy = p.y - player_y;
				dist2 = dx * dx + dy * dy + config::kEps;
				if (dist2 < barrier_radius2) {
					dist = std::sqrt(dist2);
					inv_dist = 1.0f / dist;
					nx = dx * inv_dist;
					ny = dy * inv_dist;
					float penetration = inf_radius - dist;
					float correction = penetration * inf_push * config::kDt;
					if (correction > penetration) {
						correction = penetration;
					}
					p.x += nx * correction;
					p.y += ny * correction;
				}
			}
		}
		
		// if a particle hasnt been affected by this point they should just continue with the path routing
		if (!destroyed) {
			route_phase_[i] += route_omega_[i] * config::kDt;
			float target_x = route_center_x_[i] + std::cos(route_phase_[i]) * route_radius_[i];
			float target_y = route_center_y_[i] + std::sin(route_phase_[i]) * route_radius_[i];
			p.vx += (target_x - p.x) * config::kRoutePull * config::kDt;
			p.vy += (target_y - p.y) * config::kRoutePull * config::kDt;

			p.vx *= config::kGlobalDamp;
			p.vy *= config::kGlobalDamp;

			float v2 = p.vx * p.vx + p.vy * p.vy;
			if (v2 > config::kMaxSpeed * config::kMaxSpeed) {
				float inv = config::kMaxSpeed / std::sqrt(v2);
				p.vx *= inv;
				p.vy *= inv;
			}

			p.x += p.vx * config::kDt;
			p.y += p.vy * config::kDt;

			if (p.x < 0.0f) {
				p.x += width_;
			} else if (p.x >= width_) {
				p.x -= width_;
			}
			if (p.y < 0.0f) {
				p.y += height_;
			} else if (p.y >= height_) {
				p.y -= height_;
			}
		}

		size_t out_idx = static_cast<size_t>(i) * 2u;
		render_buffer_[out_idx] = p.x;
		render_buffer_[out_idx + 1u] = p.y;
	}

	size_t offset = static_cast<size_t>(num_particles_) * 2u;
	for (int i = 0; i < config::kMaxProjectiles; ++i) {
		if (i < proj_count) {
			render_buffer_[offset + i * 3u] = proj_ptr[i].x;
			render_buffer_[offset + i * 3u + 1u] = proj_ptr[i].y;
			render_buffer_[offset + i * 3u + 2u] = static_cast<float>(proj_ptr[i].type);
		} else {
			render_buffer_[offset + i * 3u] = -1.0f;
			render_buffer_[offset + i * 3u + 1u] = -1.0f;
			render_buffer_[offset + i * 3u + 2u] = 0.0f;
		}
	}

	++frame_;
}
