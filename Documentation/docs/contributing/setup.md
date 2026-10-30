# Development setup

## Clone

```bash
git clone git@github.com:GerardPolloRebozado/r-type.git
cd r-type
```

## Dependencies

See [Installation](../start/installation.md). You need CMake 3.28+, a C++23 compiler,
Git, pkg-config, Lua 5.4, and the platform GL/X11/Wayland/audio libraries.

## Configure and build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
```

The first configure downloads CPM packages into `.cpm_cache/`.

## Running

```bash
./build/RTypeEditor
./build/Game
./build/Server
./build/Tests
```

## Code formatting

The project uses clang-format. Run `cmake --build build --target format` if available, or
format manually with the repository's `.clang-format`.

## See also

- [Testing](testing.md)
- [Style](style.md)
- [Engine architecture](architecture-engine.md)
- [Editor architecture](architecture-editor.md)
