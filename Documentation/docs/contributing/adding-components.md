# Adding components and systems

## Add a component

1. Define the struct in `Engine/include/Components/`.
2. Register it in a plugin: `registry.RegisterComponent<T>()`.
3. Add reflection in `Engine/include/Reflection/ComponentReflect.hpp` if it should serialize.
4. Add an `IPluginEditor` if it needs custom inspector widgets.
5. Add a Lua binding if scripts need to touch it.

## Add a system

1. Define the class in `Engine/include/Systems/` and `Engine/src/Systems/`.
2. Register it in a plugin: `registry.RegisterSystem<T>()`.
3. Document ordering constraints: register dependencies first.

## Guidelines

- Keep components dumb and small.
- One system, one job.
- Queue destruction; never mutate a container you are iterating.
- Prefer several small components over one large one.
- Bundle registration in a plugin.

## See also

- [Engine architecture](architecture-engine.md)
- [Components](../engine/components.md)
- [Systems](../engine/systems.md)
- [Plugins](../plugins/engine-plugins.md)
- [Editor plugins](../plugins/editor-plugins.md)
