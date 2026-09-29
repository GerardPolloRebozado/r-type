# CONTRIBUTING GUIDELINES

## Conventional Commits Guide

This project follows a [Conventional Commits](https://www.conventionalcommits.org/) structure to maintain a clean and readable git history. Every commit must follow this format:

```text
<type>(<scope>/<sub-scope>): <short summary>

[optional body]
```

## 1. Types (The Initial Keyword)
These keywords describe the *intention* of the commit:

* **`feat`**: A new feature (e.g., adding a new ECS system, a new UI element).
* **`fix`**: A bug fix.
* **`refactor`**: A code change that neither fixes a bug nor adds a feature (e.g., renaming variables, extracting functions).
* **`perf`**: A code change that improves performance (e.g., optimizing the main loop).
* **`style`**: Changes that do not affect the meaning of the code (formatting, removing white-space, changing `#pragma once` to `#ifndef`).
* **`docs`**: Documentation only changes (e.g., updating README, MkDocs, Doxygen comments).
* **`test`**: Adding missing tests or correcting existing tests.
* **`build`**: Changes that affect the build system or external dependencies (CMake, vcpkg).
* **`ci`**: Changes to our CI configuration files and scripts (GitHub Actions).
* **`chore`**: Other changes that don't modify `src` or `test` files (e.g., updating `.gitignore`).

## 2. Scopes (The Main Field)
The scope indicates the high-level module or component that the commit affects:

* **`engine`**: Core engine library (`libEngine.a`), ECS, core systems.
* **`editor`**: The graphical scene editor (ImGui).
* **`game`**: The specific R-Type game implementation (`Games/R-Type`).
* **`docs`**: Project documentation (MkDocs, Doxygen, PDFs).
* **`tooling`**: Scripts, CI/CD, or build configurations that affect the whole project.

## 3. Sub-Scopes (The Subsections)
To provide even more clarity in a massive project, scopes can be divided into sub-scopes using a forward slash `/`.

### Engine Sub-scopes (`engine/`)
* **`engine/ecs`**: Core registry, entity management, component arrays.
* **`engine/network`**: Asio UDP sockets, packet serialization, server logic.
* **`engine/pluginName`**: Collision detection, transforms, audio...
* **`engine/render`**: Raylib integration, sprite drawing, window management.
* **`engine/scripting`**: Lua integration, binding generation.
* **`engine/core`**: Main application loop, delta time calculation, configuration.

### Editor Sub-scopes (`editor/`)
* **`editor/ui`**: ImGui layouts, styling, custom widgets.
* **`editor/inspector`**: Component tweaking windows.
* **`editor/hierarchy`**: Entity list and selection logic.
* **`editor/build`**: Game-building related process from the editor.
* **`editor/serialize`**: Scene saving/loading to JSON.

### Game Sub-scopes (`game/`)
* **`game/rtype`**: Specific gameplay logic (shooting, alien AI).
* **`game/gameName`**: Ideally, we would have a bunch of game prototypes as

## Examples of Perfect Commits

* `feat(engine/network): implement UDP packet serialization for entity spawning`
* `fix(game/rtype): prevent aliens from moving off-screen`
* `refactor(engine/ecs): switch from vector to sparse set for component arrays`
* `style(engine/core): replace pragma once with ifndef include guards`
* `build(editor/ui): add imgui bindings to vcpkg dependencies`
* `docs(tooling): update CONTRIBUTING.md with commit guidelines`
