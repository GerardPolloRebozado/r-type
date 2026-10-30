# Packaging plugins

Engine and editor plugins are C++ code. Distributing one means shipping its source and
CMake entries, or a prebuilt static library plus headers.

## Engine plugin

1. Add the plugin's source to `Engine/CMakeLists.txt` (or use `create plugin`).
2. Include the header in the game's `main.cpp` and call `Init()`.
3. Document registration order and dependencies.

## Editor plugin

1. Add the source to `Editor/CMakeLists.txt`.
2. Register the editor in `InspectorPanel`.
3. Keep the component's reflection in sync so scenes serialize.

## Lua plugin

Ship the `.lua` files in the project's `scripts/` folder. Paths are project-relative.

## See also

- [Command line](../editor/cli.md)
- [Engine plugins](engine-plugins.md)
- [Editor plugins](editor-plugins.md)
- [Lua plugins](lua.md)
