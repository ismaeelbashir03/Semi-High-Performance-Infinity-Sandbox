import struct

from config import INIT_MAGIC, NUM_PARTICLES

INIT_FMT = "<IIII"
INPUT_FMT = "<IffIffIffffff"
PARTICLE_FMT = "<ff"
PROJECTILE_FMT = "<fff"

INIT_STRUCT = struct.Struct(INIT_FMT)
INPUT_STRUCT = struct.Struct(INPUT_FMT)
PARTICLE_STRUCT = struct.Struct(PARTICLE_FMT)
PROJECTILE_STRUCT = struct.Struct(PROJECTILE_FMT)


def pack_init(width, height):
    return INIT_STRUCT.pack(INIT_MAGIC, width, height, NUM_PARTICLES)


def pack_input(technique, player_x, player_y, fire, dir_x, dir_y, barrier_on, tuning):
    return INPUT_STRUCT.pack(
        technique,
        float(player_x),
        float(player_y),
        1 if fire else 0,
        float(dir_x),
        float(dir_y),
        1 if barrier_on else 0,
        float(tuning.radius),
        float(tuning.power),
        float(tuning.jitter),
        float(tuning.push),
        float(tuning.projectile_speed),
        float(tuning.projectile_field),
    )


def iter_particles(buffer):
    return PARTICLE_STRUCT.iter_unpack(buffer)


def iter_projectiles(buffer):
    return PROJECTILE_STRUCT.iter_unpack(buffer)
