# Command line

`RTypeEditor` can create a project, a system, or a plugin from the terminal. The same functions live in `Editor::CreateProject`, `Editor::CreateSystem`, and `Editor::CreatePlugin` (`Editor/include/Scaffold.hpp`). A future editor button should call those functions instead of writing files itself.

With no arguments, the Gallery opens as usual.

## Create a project

```bash
./RTypeEditor new <name> [--path <dir>] [--description <text>]
```

`<name>` is the window title. `--path` is the project directory. When `--path` is omitted, the directory is `./<name>` in the current working directory. `--description` is stored in the Gallery list.

The command writes:

- `test.scene`, an empty scene the editor can open
- `assets/`
- `input.json`

It also adds the project to the Gallery list. It then opens the editor on `test.scene`. The Gallery window is not shown.

The scene uses the project's name as its title, a 1280×720 window, 60 FPS, and a build executable name equal to the project name with spaces removed. `main.cpp` is not written. The editor generates that file when you build.

If `test.scene` already exists, the command prints an error and does not open the editor. A project path that is already in the Gallery list is not added again.

## Create a system

```bash
./RTypeEditor create system <Name> [--path <dir>]
```

`<Name>` is a C++ identifier. `System` is appended when the name does not already end with it, so `Damage` and `DamageSystem` both create `DamageSystem`.

`--path` is the engine root (the folder that contains `CMakeLists.txt`, `include/`, and `src/`). When `--path` is omitted, that folder is the **current working directory** (run from `Engine/` or pass `--path Engine` from the repository root).

The command writes the header and source under that tree and adds them to its `CMakeLists.txt`:

- `include/Systems/<Name>System.hpp`
- `src/Systems/<Name>System.cpp`

It does not open the editor, and it does not register the system. Call `RegisterSystem` from a plugin `Init` when you want it to run. Existing files are left unchanged.

## Create a plugin

```bash
./RTypeEditor create plugin <Name> [--path <dir>]
```

`<Name>` is a C++ identifier. `Plugin` is appended when the name does not already end with it. `--path` works the same as for `create system`. `Health` and `HealthPlugin` both create:

- `include/Components/Health.hpp`
- `include/Systems/HealthSystem.hpp`
- `src/Systems/HealthSystem.cpp`
- `include/Plugins/HealthPlugin.hpp`
- `src/Plugins/HealthPlugin.cpp`

`HealthPlugin::Init` registers the component and the system. The command does not open the editor. Call `Init` from game code to use the plugin. Existing files are left unchanged.

## Exit status

A missing name, an unknown command, or an existing file prints an `[ERROR]` line and returns `1`. Success prints an `[INFO]` line and returns `0`. `create` returns after writing the files. `new` returns when the editor window closes.

## Automated and manual tests

Run the automated scaffold checks after building:

```bash
cmake --build build --target ScaffoldRoundTrip
./build/ScaffoldRoundTrip
```

For a safe walkthrough that keeps everything under `/tmp` (including a fake `Engine/` tree via `--path`), run `bash Tests/scaffold_cli_tmp.sh` from the repository root. The script keeps the project folder so it still appears in the Gallery after you quit the editor. Re-launch `./build/RTypeEditor` with no arguments to see the list.

To exercise the real `Engine/CMakeLists.txt` and generated sources in the repo, use `bash Tests/manual_scaffold_cli.sh` instead and remove the files it lists when you are done.
