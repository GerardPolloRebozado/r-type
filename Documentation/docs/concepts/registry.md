# The Registry

The **Registry** is the center of the R-Type Engine's Entity Component System (ECS).

## Key Responsibilities

1. **Entity Management**: It generates unique IDs for new Entities and safely queues destroyed entities to be removed at the end of the frame (`FlushQueues()`).
2. **Component Storage**: It manages `ComponentArray`s behind the scenes, making sure that components of the same type are packed together in memory for cache-friendly iteration.
3. **System Registration**: It holds instances of all Systems and provides them with the data they need to run every frame.
4. **Resource Management**: It stores global singleton-like data (such as the `AssetManager` or `State` enums) so they can be accessed anywhere without global variables.

## Usage Example

```cpp
// 1. Spawning an entity
Engine::Entity player = registry.CreateEntity();

// 2. Attaching a component
registry.AddComponent(player, Transform{ .x = 100, .y = 200 });

// 3. Fetching a resource
auto& assets = registry.GetResource<Engine::AssetManager>();
```
