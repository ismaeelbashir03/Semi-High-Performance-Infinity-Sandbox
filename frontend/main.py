import math
import sys
import time

import pygame

from config import (
    HEIGHT,
    MAX_PROJECTILES,
    NUM_PARTICLES,
    PLAYER_SPEED,
    SCROLL_RADIUS_STEP,
    TECH_BLUE,
    TECH_PURPLE,
    TECH_RED,
    WIDTH,
)
from net import NetworkClient
from protocol import pack_input
from renderer import Renderer
from ui import SettingsMenu, TuningConfig


def clamp(value, min_value, max_value):
    return max(min_value, min(value, max_value))


def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT), pygame.SCALED | pygame.DOUBLEBUF)
    pygame.display.set_caption("Project Limitless")

    clock = pygame.time.Clock()
    font = pygame.font.Font(None, 22)
    font_small = pygame.font.Font(None, 18)

    pygame.event.set_allowed(
        [pygame.QUIT, pygame.KEYDOWN, pygame.MOUSEBUTTONDOWN, pygame.MOUSEWHEEL]
    )

    renderer = Renderer(screen)
    tuning = TuningConfig()
    menu = SettingsMenu(tuning)

    client = NetworkClient()
    try:
        client.connect()
    except OSError as exc:
        print(f"Failed to connect to backend: {exc}")
        return 1

    particle_bytes = NUM_PARTICLES * 2 * 4
    projectile_bytes = MAX_PROJECTILES * 3 * 4
    frame_buffer = bytearray(particle_bytes + projectile_bytes)
    mv = memoryview(frame_buffer)

    technique = TECH_BLUE
    barrier_on = False
    player_x = WIDTH * 0.5
    player_y = HEIGHT * 0.5

    running = True
    last_caption = time.time()

    while running:
        dt = clock.tick(60) / 1000.0
        fire = False

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    menu.toggle()
                elif event.key == pygame.K_q:
                    running = False
                elif event.key == pygame.K_1:
                    technique = TECH_BLUE
                elif event.key == pygame.K_2:
                    technique = TECH_RED
                elif event.key == pygame.K_3:
                    technique = TECH_PURPLE
                elif event.key == pygame.K_SPACE:
                    barrier_on = not barrier_on
                elif menu.open:
                    menu.handle_key(event)
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1 and not menu.open:
                    fire = True
            elif event.type == pygame.MOUSEWHEEL:
                menu.adjust_radius(event.y * SCROLL_RADIUS_STEP)

        keys = pygame.key.get_pressed()
        if not menu.open:
            move_x = keys[pygame.K_d] - keys[pygame.K_a]
            move_y = keys[pygame.K_s] - keys[pygame.K_w]
            if move_x or move_y:
                move_len = math.hypot(move_x, move_y)
                move_x /= move_len
                move_y /= move_len
                player_x = clamp(player_x + move_x * PLAYER_SPEED * dt, 0.0, WIDTH)
                player_y = clamp(player_y + move_y * PLAYER_SPEED * dt, 0.0, HEIGHT)

        aim_x, aim_y = pygame.mouse.get_pos()
        dir_x = aim_x - player_x
        dir_y = aim_y - player_y
        length = math.hypot(dir_x, dir_y)
        if length > 1e-4:
            dir_x /= length
            dir_y /= length
        else:
            dir_x = 0.0
            dir_y = 0.0

        input_msg = pack_input(technique, player_x, player_y, fire, dir_x, dir_y, barrier_on, tuning)
        try:
            client.send_and_receive(input_msg, frame_buffer)
        except (OSError, ConnectionError) as exc:
            print(f"Connection lost: {exc}")
            running = False
            break

        renderer.draw( mv[:particle_bytes], mv[particle_bytes:], (player_x, player_y), barrier_on, tuning.radius, technique)
        if menu.open:
            menu.draw(screen, font, font_small, barrier_on)

        pygame.display.flip()
        now = time.time()
        if now - last_caption > 0.5:
            pygame.display.set_caption(f"Project Limitless - FPS: {clock.get_fps():.1f}")
            last_caption = now

    client.close()
    pygame.quit()
    return 0


if __name__ == "__main__":
    sys.exit(main())
