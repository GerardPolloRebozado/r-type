# Scene serialization

A scene is a snapshot of the registry: entities, their components, and the project
configuration. The editor writes it; the game reads it at startup.

```mermaid
flowchart LR
    R[Registry] -->|CaptureScene| S[SceneSnapshot]
    S -->|JSON / BEVE| F[".scene / .prefab"]
    F -->|RestoreScene| R2[Registry]
```

---

## Using the serializer

```cpp
#include "Serialization/SceneSerializer.hpp"

Serialization::SceneSerializer serializer;
Serialization::SceneConfig config;

// Save the current world.
serializer.Serialize("test.scene", registry, config);

// Load it back.
if (!serializer.Deserialize("test.scene", registry, config)) {
    log_error("Could not read test.scene");
}
```

`Deserialize` returns `false` when the file cannot be read. `config` is optional; pass it
when you also want to restore the window, virtual resolution, input map, and build settings.

The default format is JSON. `SerializationFormat::Binary` writes BEVE (glaze's compact
binary format) with the same snapshot structure.

---

## What is stored

Each entity is captured as an optional block per component:

| Field | Component |
|---|---|
| `name` | `Name` |
| `position` | `Position` |
| `rotation` | `Rotation` |
| `sprite` | `SpriteComponent` |
| `shape` | `ShapeComponent` |
| `text` | `Text` |
| `collider` | `ColliderComponent` |
| `opacity` | `Opacity` |
| `connections` | `SignalConnections` |
| `parent` | `Parent` |
| `input` | `Input` |
| `audioSource` | `AudioSource` |
| `camera` | `CameraComponent` |
| `script` | `ScriptComponent` |
| `network` | `NetworkComponent` |
| `prefab` | `PrefabInstance` (subtree roots only) |

A component that is absent is simply not attached. This keeps files forward-compatible:
adding a component to the engine does not invalidate old scenes.

!!! note "IDs are not stable"
    The `entityId` field is useful for debugging, but loading assigns fresh IDs and remaps
    `Parent` and `SignalConnections` references. Never key gameplay logic off a saved ID.

---

## Reflection

Components opt into serialization through `Engine/include/Reflection/ComponentReflect.hpp`
(glaze metadata). If a component is missing there, the editor cannot save it.

When you add a component:

1. Give it public fields with stable names.
2. Add a `glz::meta<T>` specialization with the fields to persist.
3. Register it in a plugin so the editor knows the type.
4. Add an `IPluginEditor` if it needs custom inspector widgets.

---

## Scenes vs. prefabs

| | `.scene` | `.prefab` |
|---|---|---|
| Scope | Whole registry + config | One entity subtree |
| Edited in | Scene View / Hierarchy | Preview panel |
| Instances | One | Many, sharing component data |
| Reference | — | `PrefabInstance` on the root |

A prefab is not a separate runtime concept: instantiating one copies the subtree and records
the prefab path. Editing the prefab through Preview updates every instance that still
references it; per-instance positions are preserved.

---

## Format notes

- Files are human-readable JSON by default; diff them before committing.
- Asset paths are project-relative so scenes work in the editor and the built game.
- The serializer is versionless today. Treat removed or renamed fields as a breaking change
  and add a migration before shipping.

See also: [Scene serialization API](../api/index.md) · [Command line](cli.md) ·
[Runtime prefabs](interface.md#copy-paste-and-prefabs).
