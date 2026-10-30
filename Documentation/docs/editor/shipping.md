# Shipping a game

Shipping an editor-made game means producing a runnable executable that loads the scene
you authored.

## Build

Click **Build** (hammer) in the editor, or build from the command line. The build writes
`main.cpp` and a `.editor_build.cmake`, configures CMake with `EDITOR_BUILD=ON`, and
compiles the executable named in the project settings (`m_BuildExecutableName`).

The executable is placed in the project folder, or in `build/`, `build/<Config>/`, or
`build/Games/R-Type/<Config>/` depending on the generator.

## Running

The game takes the scene to start with as an argument:

```bash
./MyGame levels/Menu.scene
```

With no argument it uses the startup scene from `project.json`. Multiplayer roles come
from flags:

```bash
./MyGame --server --headless
./MyGame --client --host 10.0.0.5 --port 4000
```

## Assets

Bundled assets are compiled into the executable by `AssetBundler`, so a shipped game does
not need its `assets/` folder at runtime. Disk-loaded assets still need their files.

## Multiplayer

Ship the server and client binaries together, or run a dedicated server with
`--server --headless`. See [Multiplayer](multiplayer.md).

## See also

- [Play mode](play.md)
- [Scenes](scenes.md)
- [Assets](../engine/assets.md)
- [Networking](../engine/networking.md)
