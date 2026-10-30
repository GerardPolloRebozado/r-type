# Engine architecture

## Layers

- **Core**: `Application`, `Registry`, `VirtualResolution`, rendering, logging, errors.
- **ECS**: entities, components, systems, resources.
- **Plugins**: bundles of components, systems, and resources behind one `Init()`.
- **Assets**: `AssetManager`, bundler, project paths.
- **Scripting**: `ScriptComponent`, `ScriptSystem`, Lua API.
- **Networking**: replication and multiplayer plugins on top of ENet.
- **Serialization**: scene and prefab files (separate module).

## Where to add code

- New data: a component struct in `Engine/include/Components`.
- New behaviour: a system in `Engine/include/Systems` and `Engine/src/Systems`.
- New feature: a plugin in `Engine/include/Plugins` and `Engine/src/Plugins`.
- New Lua binding: a module under `Engine/include/Scripting/LuaAPI`.

Register components and systems in a plugin rather than in `main()`. Keep `main()` to
plugin instantiation and entity spawning.

## Ordering constraints

- `HierarchyPlugin` before `PositionPlugin`.
- `ReplicationPlugin` before `MultiplayerPlugin`.
- Systems register in dependency order; registration order is update order.

## See also

- [Registry](../engine/registry.md) · [Components](../engine/components.md) · [Systems](../engine/systems.md)
- [Plugins](../plugins/engine-plugins.md)
- [Adding components and systems](adding-components.md)
- [Editor architecture](architecture-editor.md)
