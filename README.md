# Infinity Simulator/Game/Sandbox

High performance (attempt) at a particle sandbox that mimicks gojos limitless technique from jjk.
Using C++ with OpenMP for the phsycics engine and Python with PyGame (bottleneck) for rendering.

## Build (C++)

```sh
cd backend
make
```

If you are using mac, install libimp with brew:

```sh
brew install libomp
```

## Run

Start the backend:

```sh
./backend/limitless_server
```

Run the frontend:

```sh
python3 frontend/main.py
```

## Controls

- `WASD`: move the player
- `1` Blue projectile (absorbs)
- `2` Red projectile (repels)
- `3` Purple projectile (erases)
- Mouse left click: fire selected technique toward cursor
- `Space`: toggle Infinity barrier
- Mouse wheel: adjust barrier radius
- `Esc`: open/close settings menu (Infinity tuning)
- `Q`: quit

## Tweaking

- Adjust `NUM_PARTICLES` in `frontend/config.py` to scale the load
- Use the menu (esc) in game to tweak the infinity radius, damping power, jitter, push force, and projectile tuning live.

## Performance

Backend:
- OpenMP parallelises the particle update loop with static scheduling - this is best since the workload is uniform
- The particle data is stored in flat arrays, so we get cache friendly access
- I send the frames as a single binary blob, reducing the syscall overhead we get with all the sends

Frontend (Python/PyGame):
- I use `struct.Struct` and `memoryview` to avoid per particle allocations and copying when unpacking data from backend
- We render with prebuilt surfaces for fast blits (no per frame surface creation)
- The python renderer is still the main bottleneck :( , but i cba to rewrite it in c++ right now
