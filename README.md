# Lucky & Pozzo

A curious partnership in game-making: **Pozzo**, a modern C++23 game engine, and **Lucky**, the editor that drives it.

## Highlights

- **ECS core** — registry, components, systems, resources, and entity lifecycle
- **Rendering & audio** — raylib-backed 2D rendering, fonts, sprites, particles, audio
- **Networking** — lockstep turn-based multiplayer over ENet, with a dedicated multi-room server
- **Scripting** — Lua 5.4 via sol2, with lifecycle hooks and a component API
- **Editor** — scene editor, hierarchy/inspector, prefabs, play mode, asset bundler
- **Plugins** — engine, editor, and Lua plugins with a documented lifecycle

## Quick start

### Requirements

- CMake 3.28+
- C++23 compiler (GCC 13+, Clang 17+, or MinGW-w64 GCC 13+)
- Lua 5.4 development headers
- Git, pkg-config
- Platform libs (OpenGL, X11/Wayland, audio) — see [installation docs](Documentation/docs/start/installation.md)

### Build

```bash
git clone https://github.com/GerardPolloRebozado/r-type.git
cd r-type
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Run the editor

```bash
./build/RTypeEditor
```

Open a project from the Gallery or create a new one. The tracked R-Type sample can be opened directly:

```bash
./build/RTypeEditor R-Type/test.scene
```

### Build targets

| Target | Description |
|---|---|
| `RTypeEditor` | Editor and project gallery |
| `Game` | R-Type sample target |
| `Openfront` | Openfront RTS (single/multiplayer) |
| `Server`, `Client` | Low-level multiplayer example (port 3000) |
| `AssetBundler` | Asset packing tool |
| `SerializationRoundTrip`, `PacketRoundTrip`, `ScaffoldRoundTrip` | Test executables |

## Documentation

Full documentation is published at [lucky-n-pozzo.pollorebozado.com](https://lucky-n-pozzo.pollorebozado.com/) and lives under `Documentation/` (MkDocs Material). Serve it locally:

```bash
cd Documentation
pip install -r requirements.txt
mkdocs serve
```

## Repository layout

```
Engine/        Core engine (ECS, rendering, audio, networking, scripting)
Editor/        Lucky editor (UI, panels, play mode, bundler)
Games/         Sample games and shipped titles (R-Type, Openfront, StreetFighter)
Openfront/     Openfront RTS (simulation, network, renderer, tests)
R-Type/        R-Type sample project (scenes, assets, scripts)
Serialization/ Serialization library
Tools/         Helper scripts (multi-room server, smoke tests)
Documentation/ MkDocs site
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for commit conventions and [DEVELOPMENT.md](DEVELOPMENT.md) for local setup. Documentation contributions are welcome under `Documentation/docs/`.

## License

This project is released under the [MIT License](LICENSE). Third-party libraries retain their own licenses (see `Engine/third_party`, `Editor/third_party`).
