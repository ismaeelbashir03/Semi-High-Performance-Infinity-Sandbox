import pygame

from config import (
    COLOUR_BG,
    COLOUR_BLUE,
    COLOUR_INF_IN,
    COLOUR_INF_OUT,
    COLOUR_PLAYER,
    COLOUR_PURPLE,
    COLOUR_RED,
    PARTICLE_RADIUS,
    PLAYER_RADIUS,
    PROJECTILE_RADIUS_BLUE,
    PROJECTILE_RADIUS_PURPLE,
    PROJECTILE_RADIUS_RED,
    TECH_BLUE,
    TECH_PURPLE,
    TECH_RED,
)
from protocol import iter_particles, iter_projectiles


def make_particle_surface(radius, colour):
    size = radius * 2 + 1
    surf = pygame.Surface((size, size), pygame.SRCALPHA)
    pygame.draw.circle(surf, colour, (radius, radius), radius)
    return surf


class Renderer:
    def __init__(self, screen):
        self.screen = screen
        self.surf_inf_out = make_particle_surface(PARTICLE_RADIUS, COLOUR_INF_OUT)
        self.surf_inf_in = make_particle_surface(PARTICLE_RADIUS, COLOUR_INF_IN)
        self.surf_proj_blue = make_particle_surface(int(PROJECTILE_RADIUS_BLUE), COLOUR_BLUE)
        self.surf_proj_red = make_particle_surface(int(PROJECTILE_RADIUS_RED), COLOUR_RED)
        self.surf_proj_purple = make_particle_surface(int(PROJECTILE_RADIUS_PURPLE), COLOUR_PURPLE)
        self.surf_player = make_particle_surface(PLAYER_RADIUS, COLOUR_PLAYER)

    def draw(self, particles_view, projectiles_view, player_pos, barrier_on, barrier_radius, technique):
        screen = self.screen
        screen.fill(COLOUR_BG)

        particle_surf = self.surf_inf_out

        barrier2 = barrier_radius * barrier_radius if barrier_on else 0.0
        player_x, player_y = player_pos

        for x, y in iter_particles(particles_view):
            ix = int(x)
            iy = int(y)
            if ix < -PARTICLE_RADIUS or iy < -PARTICLE_RADIUS:
                continue
            if barrier_on:
                dx = x - player_x
                dy = y - player_y
                if dx * dx + dy * dy < barrier2:
                    screen.blit(self.surf_inf_in, (ix - PARTICLE_RADIUS, iy - PARTICLE_RADIUS))
                else:
                    screen.blit(particle_surf, (ix - PARTICLE_RADIUS, iy - PARTICLE_RADIUS))
            else:
                screen.blit(particle_surf, (ix - PARTICLE_RADIUS, iy - PARTICLE_RADIUS))

        if barrier_on:
            pygame.draw.circle(
                screen,
                COLOUR_INF_IN,
                (int(player_x), int(player_y)),
                int(barrier_radius),
                1,
            )

        screen.blit(self.surf_player, (int(player_x) - PLAYER_RADIUS, int(player_y) - PLAYER_RADIUS))

        for px, py, ptype in iter_projectiles(projectiles_view):
            if px >= 0.0 and py >= 0.0:
                if int(ptype) == TECH_BLUE:
                    surf = self.surf_proj_blue
                    radius = PROJECTILE_RADIUS_BLUE
                elif int(ptype) == TECH_RED:
                    surf = self.surf_proj_red
                    radius = PROJECTILE_RADIUS_RED
                else:
                    surf = self.surf_proj_purple
                    radius = PROJECTILE_RADIUS_PURPLE
                screen.blit(
                    surf,
                    (int(px) - int(radius), int(py) - int(radius)),
                )
