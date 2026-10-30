# Scenes

A scene is a snapshot of the registry: entities, their components, and the project
configuration. The editor writes it; the game reads it at startup.

## Save and load

Save with ++ctrl+s++ or **File → Save Scene**. The file is written to
`m_ActiveScenePath`. **Save Scene As...** writes to a new path and switches to it.

The game deserializes the same file at startup through `Serialization::SceneSerializer`.
`Deserialize` returns `false` when the file cannot be read.

## Startup scene

Each project has a startup scene, named in `project.json`:

```json
{"startupScene": "levels/Menu.scene"}
```

The game starts with that scene. In the editor, **File → Set as Startup Scene** marks the
active scene. Projects without `project.json` default to `test.scene`.

## Scene files vs. prefabs

| | `.scene` | `.prefab` |
|---|---|---|
| Scope | Whole registry + config | One entity subtree |
| Edited in | Scene view / Hierarchy | Preview panel |
| Instances | One | Many, sharing component data |
| Reference |  -  | `PrefabInstance` on the root |

## Format

Scenes are human-readable JSON by default; `SerializationFormat::Binary` writes BEVE with
the same snapshot structure. Asset paths are project-relative. See
[Reference → Scene format](../editor/scenes.md).

## New scene

**File → New Scene...** creates an empty scene in the project folder. Scene switching
inside the editor asks before discarding unsaved changes.

## See also

- [Interface](interface.md)
- [Prefabs](prefabs.md)
- [Command line](cli.md)
- [Scene format](../editor/scenes.md)
