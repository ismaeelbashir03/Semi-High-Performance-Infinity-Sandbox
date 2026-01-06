from dataclasses import dataclass

import pygame

from config import (
    COLOUR_PANEL,
    COLOUR_PANEL_BORDER,
    COLOUR_PANEL_HILITE,
    COLOUR_PANEL_TEXT,
    DEFAULT_INF_JITTER,
    DEFAULT_INF_POWER,
    DEFAULT_INF_PUSH,
    DEFAULT_INF_RADIUS,
    DEFAULT_PROJECTILE_FIELD,
    DEFAULT_PROJECTILE_SPEED,
    PROJECTILE_FIELD_MAX,
    PROJECTILE_FIELD_MIN,
    PROJECTILE_SPEED_MAX,
    PROJECTILE_SPEED_MIN,
)


@dataclass
class TuningConfig:
    radius: float = DEFAULT_INF_RADIUS
    power: float = DEFAULT_INF_POWER
    jitter: float = DEFAULT_INF_JITTER
    push: float = DEFAULT_INF_PUSH
    projectile_speed: float = DEFAULT_PROJECTILE_SPEED
    projectile_field: float = DEFAULT_PROJECTILE_FIELD

    def reset(self):
        self.radius = DEFAULT_INF_RADIUS
        self.power = DEFAULT_INF_POWER
        self.jitter = DEFAULT_INF_JITTER
        self.push = DEFAULT_INF_PUSH
        self.projectile_speed = DEFAULT_PROJECTILE_SPEED
        self.projectile_field = DEFAULT_PROJECTILE_FIELD


@dataclass
class Setting:
    label: str
    attr: str
    step: float
    min_value: float
    max_value: float
    fmt: str


class SettingsMenu:
    def __init__(self, config):
        self.config = config
        self.settings = [
            Setting("Infinity radius", "radius", 5.0, 20.0, 200.0, "{:.0f}px"),
            Setting("Infinity damping power", "power", 0.2, 0.5, 6.0, "{:.2f}"),
            Setting("Infinity jitter", "jitter", 0.1, 0.0, 5.0, "{:.2f}px"),
            Setting("Infinity push", "push", 100.0, 0.0, 5000.0, "{:.0f}"),
            Setting("Projectile speed", "projectile_speed", 0.05, PROJECTILE_SPEED_MIN, PROJECTILE_SPEED_MAX, "{:.2f}x"),
            Setting("Projectile field strength", "projectile_field", 0.05, PROJECTILE_FIELD_MIN, PROJECTILE_FIELD_MAX, "{:.2f}x"),
        ]
        self.selected = 0
        self.open = False

    def toggle(self):
        self.open = not self.open

    def handle_key(self, event):
        if event.key == pygame.K_UP:
            self.selected = (self.selected - 1) % len(self.settings)
            return True
        if event.key == pygame.K_DOWN:
            self.selected = (self.selected + 1) % len(self.settings)
            return True
        if event.key in (pygame.K_LEFT, pygame.K_RIGHT):
            setting = self.settings[self.selected]
            step = setting.step
            if event.mod & pygame.KMOD_SHIFT:
                step *= 5.0
            delta = -step if event.key == pygame.K_LEFT else step
            value = getattr(self.config, setting.attr) + delta
            value = max(setting.min_value, min(value, setting.max_value))
            setattr(self.config, setting.attr, value)
            return True
        if event.key == pygame.K_r:
            self.config.reset()
            return True
        return False

    def adjust_radius(self, delta):
        setting = self.settings[0]
        value = getattr(self.config, setting.attr) + delta
        value = max(setting.min_value, min(value, setting.max_value))
        setattr(self.config, setting.attr, value)

    def draw(self, screen, font, font_small, barrier_on):
        panel_width = 380
        panel_height = 290
        panel = pygame.Surface((panel_width, panel_height), pygame.SRCALPHA)
        panel.fill(COLOUR_PANEL)
        pygame.draw.rect(panel, COLOUR_PANEL_BORDER, panel.get_rect(), 1)
        screen.blit(panel, (20, 20))

        title = font.render("Simulation Settings", True, COLOUR_PANEL_TEXT)
        screen.blit(title, (32, 32))
        status = "ON" if barrier_on else "OFF"
        status_text = font.render(f"Barrier: {status}", True, COLOUR_PANEL_TEXT)
        screen.blit(status_text, (32, 52))

        for idx, setting in enumerate(self.settings):
            label = setting.label
            value = setting.fmt.format(getattr(self.config, setting.attr))
            colour = COLOUR_PANEL_HILITE if idx == self.selected else COLOUR_PANEL_TEXT
            text = font.render(f"{label}: {value}", True, colour)
            screen.blit(text, (32, 76 + idx * 24))

        hint = font_small.render(
            "Up/Down select, Left/Right adjust, Shift=fast, R reset",
            True,
            COLOUR_PANEL_TEXT,
        )
        screen.blit(hint, (32, 76 + len(self.settings) * 24))
        hint2 = font_small.render(
            "Space toggles barrier, Mouse wheel adjusts radius",
            True,
            COLOUR_PANEL_TEXT,
        )
        screen.blit(hint2, (32, 76 + len(self.settings) * 24 + 18))
        hint3 = font_small.render("Esc close menu, Q quit", True, COLOUR_PANEL_TEXT)
        screen.blit(hint3, (32, 76 + len(self.settings) * 24 + 36))
