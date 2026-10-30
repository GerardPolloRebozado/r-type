# Plugin lifecycle

A plugin is a C++ class implementing `Engine::IPlugin`. Its only method is `Init(Registry&)`,
called once during startup, before the first frame.

## Discovery and loading

There is no dynamic loading. Plugins are compiled into the executable and instantiated in
`main()` (or in the editor's `RunEditor`). Registration order is update order, so the order
in which you call `Init()` matters.

## Initialization order

The generated game registers plugins in this order:

```cpp
Engine::HierarchyPlugin hierarchy;
hierarchy.Init(app.GetRegistry());

Engine::PositionPlugin position;
position.Init(app.GetRegistry());

// ... sprites, shapes, text, rotation, replication, collider, opacity,
//     signal, input, audio, script, camera
```

`HierarchyPlugin` before `PositionPlugin` is required: hierarchy propagates parent motion,
then position snapshots previous positions.

## Shutdown

`IPlugin` has no shutdown method. Systems implement `OnSceneUnloaded` for per-scene
cleanup. The `Application` destructor unloads the virtual resolution texture and closes
the window.

## See also

- [Engine plugins](engine-plugins.md)
- [Editor plugins](editor-plugins.md)
- [Lua plugins](lua.md)
