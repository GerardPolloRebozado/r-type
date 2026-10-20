# Installation

This page takes you from a fresh clone to a running editor. Pozzo (the engine) and Lucky
(the editor) build together with CMake; their third-party libraries are fetched
automatically by [CPM](https://github.com/cpm-cmake/CPM.cmake) on the first build.

!!! tip "In a hurry?"
    If your machine already has the dependencies listed below:

    ```bash
    git clone https://github.com/GerardPolloRebozado/r-type.git
    cd r-type
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --target RTypeEditor --parallel
    ./build/RTypeEditor
    ```

## Requirements

| Tool | Version | Notes |
|---|---|---|
| CMake | 3.28 or newer | Build system |
| C++ compiler | C++23 | GCC 13+, Clang 17+, or MinGW-w64 GCC 13+ |
| Git | Any recent version | Clone and update |
| pkg-config | Any recent version | Locates Lua |
| Lua | 5.4 | Development headers and the `lua5.4` pkg-config file |
| Python | 3.11 or newer | Optional, only for building this documentation |

!!! note "Supported platforms"
    Linux is the primary development platform. macOS builds are supported with the
    Xcode command-line tools and Homebrew's Lua 5.4. Windows builds use a MinGW-w64
    toolchain; the Visual Studio/MSVC toolchain has not been validated.

!!! note "The first configure needs network access"
    CMake downloads its CPM bootstrap and the dependencies on the first configure:
    raylib, raylib-cpp, Dear ImGui, rlImGui, ENet, glaze, and sol2. They are cached in
    `.cpm_cache/` and reused by later builds.

!!! note "Documentation only"
    Building this site additionally needs MkDocs Material, mkdoxy, and Doxygen. See
    [Contributing](../contributing/index.md) for the documentation workflow.

## Install system dependencies

=== "Debian / Ubuntu"

    ```bash
    sudo apt update
    sudo apt install build-essential cmake git pkg-config liblua5.4-dev \
      libgl1-mesa-dev libglu1-mesa-dev \
      libx11-dev libxcursor-dev libxinerama-dev libxrandr-dev libxi-dev libxxf86vm-dev \
      libwayland-dev wayland-protocols libxkbcommon-dev \
      libasound2-dev libpulse-dev
    ```

=== "Fedora"

    ```bash
    sudo dnf install gcc-c++ cmake git pkgconf-pkg-config lua-devel \
      mesa-libGL-devel mesa-libGLU-devel \
      libX11-devel libXcursor-devel libXinerama-devel libXrandr-devel libXi-devel libXxf86vm-devel \
      wayland-devel wayland-protocols-devel libxkbcommon-devel \
      alsa-lib-devel pulseaudio-libs-devel
    ```

    The project needs Lua **5.4**, so check the package version on your release. If
    `lua-devel` has moved to 5.5, install Lua 5.4 from source and make sure
    `pkg-config --modversion lua5.4` reports `5.4.x`.

=== "Arch Linux"

    ```bash
    sudo pacman -S --needed base-devel cmake git pkgconf lua54 \
      mesa glu libx11 libxcursor libxinerama libxrandr libxi libxxf86vm \
      wayland wayland-protocols libxkbcommon alsa-lib libpulse
    ```

=== "macOS"

    ```bash
    xcode-select --install
    brew install cmake git pkg-config lua@5.4
    ```

    Homebrew's `lua` formula now tracks Lua 5.5, so install `lua@5.4` explicitly.
    It is keg-only, so point `pkg-config` at it before configuring:

    ```bash
    export PKG_CONFIG_PATH="$(brew --prefix lua@5.4)/lib/pkgconfig:$PKG_CONFIG_PATH"
    ```

    Add that line to your shell profile if you want it in every new terminal.

=== "Windows"

    Use a MinGW-w64 toolchain. The Visual Studio/MSVC toolchain is not validated.

    Install Lua 5.4 from PowerShell:

    ```powershell
    winget install --id DEVCOM.Lua
    ```

    Install the MinGW-w64 toolchain, CMake, and pkg-config with MSYS2:

    ```bash
    pacman -S --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
      mingw-w64-x86_64-pkgconf
    ```

    Run the build commands below from the **MinGW 64-bit** shell, not the MSYS shell.
    If CMake does not select the MinGW generator automatically, add
    `-G "MinGW Makefiles"` to the configure command.

!!! tip "Nix and devenv"
    If you use [devenv](https://devenv.sh), the repository's `devenv.nix` provides
    CMake, a C++ toolchain, Lua 5.4, and the OpenGL/X11/Wayland/audio libraries:

    ```bash
    devenv shell
    ```

    With direnv installed, `direnv allow` enters the same environment automatically.

## Clone the repository

```bash
git clone https://github.com/GerardPolloRebozado/r-type.git
cd r-type
```

If you already have SSH keys configured for GitHub, the SSH URL works too:

```bash
git clone git@github.com:GerardPolloRebozado/r-type.git
```

## Configure and build

For a normal build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

For engine or editor development, use a debug build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

The project enables `CMAKE_EXPORT_COMPILE_COMMANDS` itself, so `build/compile_commands.json`
is available to clangd and other language servers without extra flags.

To rebuild only the editor after a small change:

```bash
cmake --build build --target RTypeEditor --parallel
```

`RTypeEditor` links the engine and serialization libraries, so this target builds `Engine`,
`Serialization`, and `AssetBundler` first. It skips the sample games, the example programs,
and the tests; run a plain `cmake --build build` when you want those too.

!!! tip "Keep the build tree disposable"
    The `build/` directory is ignored by Git. If a build gets into a strange state, remove
    it and configure again; the CPM cache in `.cpm_cache/` is reused.

## Run the editor

```bash
./build/RTypeEditor
```

With no arguments, the editor opens the Project Gallery. On a first run the project list
is empty: choose **New Project** to create one, or **Open** to select an existing project
folder.

To open a scene directly:

```bash
./build/RTypeEditor R-Type/test.scene
```

That opens the tracked R-Type sample; press **Play** in the editor to run it.

You can also create a project from the terminal:

```bash
./build/RTypeEditor new "My Game" --path ~/games/my-game
```

The command creates `test.scene`, `assets/`, and `input.json` in the target folder, adds the
project to the Gallery, and opens the editor. If `--path` is omitted, the project is created
in `./My Game`.

## Built targets

All executables are written to the build directory. On Windows the files end in `.exe`
(for example, `build\RTypeEditor.exe`).

| Binary | What it is |
|---|---|
| `build/RTypeEditor` | The editor and project gallery |
| `build/Game` | The R-Type sample target |
| `build/Server`, `build/Client` | Low-level multiplayer example (server listens on port 3000) |
| `build/AssetBundler` | Asset packing tool used by the build |
| `build/SerializationRoundTrip` | Serialization tests |
| `build/PacketRoundTrip` | Networking packet tests |
| `build/ScaffoldRoundTrip` | Project and scaffold tests |

The `Game` target is a build fixture for the sample. To run the R-Type project, open it in
the editor as shown above; the editor resolves its assets, scripts, and prefabs relative to
the project folder.

The repository also builds small example programs (`Example`, `Sounds`, `Sprites`, `Text`,
`Camera`, `Collisions`, `InputManager`, `Hierarchy`, and `OnStateChange`). They are
development fixtures and are not required to use the editor.

### Run the tests

```bash
./build/SerializationRoundTrip
./build/PacketRoundTrip
./build/ScaffoldRoundTrip
```

Each test executable prints its failures and returns a non-zero exit code on failure.

!!! warning "Certain executables will be removed after alpha."
    Once the project is out of the alpha development stage, only the Editor executable and the Engine will be compiled

## Troubleshooting

| Problem | Fix |
|---|---|
| `CMake 3.28 or higher is required` | Upgrade CMake. Distribution packages are sometimes older; use your package manager, Kitware's APT repository, or `pip install --upgrade cmake`. |
| `Could NOT find PkgConfig` | Install `pkg-config` (Debian/Ubuntu, macOS) or `pkgconf` (Arch, Fedora, MSYS2). |
| `Package 'lua5.4' was not found` | Install the Lua 5.4 development package and check `pkg-config --modversion lua5.4`. On macOS, set `PKG_CONFIG_PATH` as shown above. |
| `Lua 5.4 not found` on Windows | Run `winget install --id DEVCOM.Lua`, then configure from the MinGW 64-bit shell so pkg-config is on `PATH`. |
| `Could NOT find OpenGL` | Install the OpenGL and X11 development packages for your distribution. |
| Linker errors for `Xrandr`, `Xi`, or `Xcursor` | Install the matching `-dev`/`-devel` packages. Do not use a Wayland-only package set. |
| CPM fails to download a dependency | Check your network and proxy settings. Set `HTTPS_PROXY` if needed, or point `CPM_SOURCE_CACHE` at a shared cache. |
| The Gallery is empty | That is expected on a fresh checkout. Use **New Project** or **Open** to add a project. |
| The `Game` target is missing | Do not configure with `-DEDITOR_BUILD=ON`; that flag is reserved for builds started by the editor. Reconfigure without it. |
| Black window on Wayland | Try an X11 session; raylib's Wayland support depends on the GLFW build and the compositor. |

## Next steps

- [Your first game in the editor](first-game-editor.md) — create a project and press Play.
- [Core concepts](concepts.md) — entities, components, systems, resources, and scenes.
- [Editor tour](editor-tour.md) — a quick walk through the panels.
- [Building the engine](../engine/building.md) — link Pozzo into your own C++ project.
