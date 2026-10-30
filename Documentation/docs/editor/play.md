# Play mode

The editor is an authoring tool: Lua does not run inside it, and the multiplayer plugin is
not loaded into the editor process. **Play** and **Play Multiplayer** launch the real game
binary with the current scene.

## Build

Click the hammer to compile the project. The first build generates `main.cpp` and the CMake
files if needed. Build output appears in the Console panel. Progress is shown on the
button.

## Play

Click the play triangle. The editor saves the scene first, then launches the executable
with the current scene as an argument. If the executable is missing, build first.

## Play Multiplayer

Click the people icon. The editor saves the scene, then launches one server and N clients,
each in its own window. **Stop** (which replaces the play buttons while games run) closes
every launched process.

See [Multiplayer](multiplayer.md) for roles, settings, and troubleshooting.

## Notes

- The game starts on the scene being edited.
- Script edits do not need a rebuild, but they do need a relaunch: save the file, close
  the game window, and click Play again.
- There is no hot reload in the running process.
