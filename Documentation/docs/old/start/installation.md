# Installation

This page gets you from a clone to a running engine and editor. The project builds with
CMake 3.28+ and C++23; dependencies are fetched by [CPM](https://github.com/cpm-cmake/CPM.cmake)
on the first configure.

---

## Requirements

| Tool | Version |
|---|---|
| CMake | 3.28 or newer |
| C++ compiler | C++23 support (GCC 13+, Clang 17+, MSVC 19.38+) |
| Git | Any recent version |
| Python | 3.11+ — only for the documentation site |

System libraries (Linux):

```bash
# Debian / Ubuntu
sudo apt install build-essential cmake git \
  libgl1-mesa-dev libglu1-mesa-dev \
  libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libxxf86vm-dev \
  libwayland-dev wayland-protocols libxkbcommon-dev \
  libasound2-dev libpulse-dev
```

macOS and Windows need no extra system packages; the CPM dependencies (raylib, ImGui,
ENet, glaze, sol2) carry their own.

!!! tip "Nix / devenv"
    The repository includes `devenv.nix` with every dependency pinned. If you use
    [devenv](https://devenv.sh), run `devenv shell` and skip the package list above.

---

## Clone and build

```bash
git clone git@github.com:GerardPolloRebozado/r-type.git
cd r-type

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

The first configure downloads CPM packages into `.cpm_cache/`. Later builds are offline.

To iterate on the engine, use a debug build with compile commands:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j"$(nproc)"
```

Incremental rebuild of one target:

```bash
cmake --build build --target RTypeEditor -j"$(nproc)"
```

---

## Run

| Binary | What it is |
|---|---|
| `build/RTypeEditor` | The editor and project gallery |
| `build/Game` | The R-Type game |
| `build/Server` | Headless multiplayer server |
| `build/Tests` | Engine tests |

```bash
./build/RTypeEditor
```

With no arguments the editor opens the Gallery. To open a scene directly:

```bash
./build/RTypeEditor path/to/test.scene
```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| `Could NOT find OpenGL` | Install the GL/X11 development packages above |
| Linker cannot find `Xrandr` / `Xi` | Install the matching `-dev` package; do not mix Wayland-only setups |
| CPM download fails behind a proxy | Set `HTTPS_PROXY`, or pre-populate `CPM_SOURCE_CACHE` |
| Black window on Wayland | Run with `SDL_VIDEODRIVER`/`GLFW` fallback, or launch from an X11 session |
| Editor builds but the game target is missing | Configure without `-DEDITOR_BUILD=ON`; that flag is for editor-triggered builds |

---

Next: [Your first game](first-game.md) or take the [editor tour](editor-tour.md).
