# Command line

`RTypeEditor` can create a project, a system, or a plugin from the terminal. The same
functions live in `Editor::CreateProject`, `Editor::CreateSystem`, and `Editor::CreatePlugin`.
With no arguments, the Gallery opens as usual.

## Create a project

```bash
./RTypeEditor new <name> [--path <dir>] [--description <text>]
```

`<name>` is the window title. `--path` is the project directory; when omitted it is
`./<name>` in the current working directory. `--description` is stored in the Gallery list.

The command writes `test.scene`, `project.json`, `assets/`, and `input.json`. It also adds
the project to the Gallery list and opens the editor on `test.scene`. The Gallery window is
not shown.

The scene uses the project's name as its title, a 1280×720 window, 60 FPS, and a build
executable name equal to the project name with spaces removed. `main.cpp` is not written;
the editor generates it when you build.

If `test.scene` already exists, the command prints an error and does not open the editor. A
project path already in the Gallery list is not added again.

## Create a system

```bash
./RTypeEditor create system <Name> [--path <dir>]
```

`<Name>` is a C++ identifier. `System` is appended when the name does not already end with
it, so `Damage` and `DamageSystem` both create `DamageSystem`.

`--path` is where the files are written; when omitted it is the current working directory.
The command walks upward to find `Engine/CMakeLists.txt` (or an engine tree in the current
folder) and updates that CMake so the new sources build.

- Inside the engine: `include/Systems/<Name>System.hpp`, `src/Systems/<Name>System.cpp`, and
  entries appended to `Engine/CMakeLists.txt`.
- Inside a Gallery project: `system/<Name>System.hpp`, `system/<Name>System.cpp`, plus
  `system/build.cmake` and `system/rtype_scaffold_register.hpp` for editor builds.

Rebuild after scaffolding. Existing files are left unchanged.

## Create a plugin

```bash
./RTypeEditor create plugin <Name> [--path <dir>]
```

`<Name>` is a C++ identifier. `Plugin` is appended when the name does not already end with
it. `--path` works the same as for `create system`.

In the engine tree, `Health` and `HealthPlugin` both create the usual `include/` and `src/`
layout under `Engine/`. In a Gallery project they are created under `system/` instead, with
the same CMake and registration hooks as project systems.

## Exit status

A missing name, an unknown command, or an existing file prints an `[ERROR]` line and
returns `1`. Success prints an `[INFO]` line and returns `0`. `create` returns after writing
the files. `new` returns when the editor window closes.

## Tests

```bash
cmake --build build --target ScaffoldRoundTrip
./build/ScaffoldRoundTrip
bash Tests/scaffold_integration.sh
```

`Tests/scaffold_integration.sh` exercises the CLI from a Gallery project folder and from
`Engine/`, then compiles the generated sources (including an `EDITOR_BUILD` game target).

For a safe walkthrough that keeps everything under `/tmp`, run
`bash Tests/scaffold_cli_tmp.sh` from the repository root. To exercise the real
`Engine/CMakeLists.txt`, use `bash Tests/manual_scaffold_cli.sh` and remove the files it
lists when you are done.

## See also

- [Interface](interface.md)
- [Plugins](../plugins/engine-plugins.md)
- [Editor plugins](../plugins/editor-plugins.md)
- [Scenes](scenes.md)
