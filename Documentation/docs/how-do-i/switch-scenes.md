# Switch scenes

From Lua:

```lua
Scene.load("levels/Level2.scene")
```

The switch happens at the end of the frame, after destruction is flushed. In multiplayer,
only the server's `Scene.load` counts; clients ignore their own calls and follow the server.

From C++, use the `SceneManager` resource:

```cpp
auto& scenes = registry.GetResource<Engine::SceneManager>();
scenes.RequestScene("levels/Level2.scene");
```

`ClearEntities()` removes all entities and restarts ids at 1, while keeping component types,
systems, and resources registered.

See [Engine → Application](../engine/application.md) and [Lua → Time](../lua/time.md), [Scene](../lua/scene.md), [Log](../lua/log.md).
