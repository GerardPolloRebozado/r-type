# Prefabs

A prefab is a reusable entity subtree saved as a `.prefab` file. It is not a separate
runtime concept: instantiating one copies the subtree and records the prefab path in a
`PrefabInstance` component on the root.

## Creating a prefab

Drag a Hierarchy row onto the **Assets** panel. Lucky writes `<Name>.prefab` in the folder
you are viewing. If the name exists, the next file is `Name_2.prefab`, then `Name_3.prefab`.

## Instantiating

Drag a `.prefab` from Assets onto the Hierarchy background (or a row) to instantiate it.
The new subtree is a copy; positions are preserved per instance.

## Editing shared data

Select a `.prefab` in Assets to open it in **Preview**. Changing the sprite path, scale,
shape size, or script there updates every instance that still references it. Positions stay
where each instance was placed, and a child position is applied as an offset from its
parent.

## Runtime spawning

The serializer installs a `PrefabSpawner` resource, so the engine can instantiate prefabs
at runtime from C++ or from networked spawn messages. There is no Lua equivalent: a Lua
script can only `Entity.create()` a bare entity and dress it with components.

## See also

- [Interface](interface.md)  -  Preview panel
- [Scenes](scenes.md)
- [Networking](../engine/networking.md)  -  prefab spawning on clients
