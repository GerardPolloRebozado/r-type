# Editor architecture

## Structure

- `EditorLayer` owns the panels and the dockspace, and bridges the engine's `onUI` and
  `customRender` callbacks.
- Panels implement `IEditorPanel` and read/write the same `Registry`.
- `EditorConfig` extends `SceneConfig` with editor-only state (selected asset, requested
  scene, script font size).
- `Gallery` manages `projects.json` and project creation.
- `Scaffold` generates project, system, and plugin files.

## Editor vs. game

The editor creates a normal `Engine::Application` with an `EditorMode` resource. `ScriptSystem`
checks for that resource and stays idle, so scripts only run in the game process. The
multiplayer plugin is not loaded into the editor process.

The editor edits a live registry and serializes it to `.scene`; the game deserializes the
same file and runs it.

## See also

- [Interface](../editor/interface.md)
- [Adding editor features](adding-editor-features.md)
- [Engine architecture](architecture-engine.md)
