# Building the engine

Pozzo builds with CMake 3.28+ and C++23. Dependencies are fetched automatically by
[CPM](https://github.com/cpm-cmake/CPM.cmake) on the first configure.

## Targets

| Target | Type | What it is |
|---|---|---|
| `Engine` | static library | Core runtime, plugins, systems |
| `Serialization` | static library | Scene and prefab serialization |
| `RTypeEditor` | executable | The Lucky editor |
| `Game` | executable | The R-Type game |
| `Server` | executable | Headless multiplayer server |
| `Tests` | executable | Engine tests |

## Configure and build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The first configure needs network access to download raylib, raylib-cpp, Dear ImGui, rlImGui,
ENet, glaze, and sol2. They are cached in `.cpm_cache/` and reused by later builds.

For development:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target RTypeEditor --parallel
```

## Options

| Option | Default | Effect |
|---|---|---|
| `EDITOR_BUILD` | `OFF` | Set by the editor when building a project; enables project-specific includes and scaffolded sources |
| `CMAKE_BUILD_TYPE` |  -  | `Debug`, `Release`, `RelWithDebInfo` |

## Using the engine in your own project

Link `Engine` and `Serialization` and include their headers:

```cmake
target_link_libraries(my_game PRIVATE Engine Serialization)
target_include_directories(my_game PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine/include
    ${CMAKE_SOURCE_DIR}/Serialization/include
)
```

The engine requires C++23. Set `CMAKE_CXX_STANDARD 23` in your project.

## Requirements

CMake 3.28+, a C++23 compiler (GCC 13+, Clang 17+, MinGW-w64 GCC 13+), Git, pkg-config,
and Lua 5.4 development headers. Linux needs the GL/X11/Wayland and audio libraries listed in
[Installation](../start/installation.md).

## See also

- [Installation](../start/installation.md)  -  system dependencies
- [Application](application.md)  -  runtime entry point
- [Plugins](../plugins/engine-plugins.md)  -  extending the engine
