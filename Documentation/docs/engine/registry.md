# Registry

`Engine::Registry` is the coordinator of the ECS. It owns entity allocation, component
storage, system registration and ordering, and global resources.

## Entities

An `Engine::Entity` is a `std::size_t`, an identifier with no data. `CreateEntity()`
allocates a new id and attaches a default `Position` and `Rotation`:

```cpp
Engine::Entity player = registry.CreateEntity();
```

Destroying an entity destroys its descendants (anything whose `Parent` points at it). Two
forms exist:

```cpp
registry.DestroyEntity(enemy);         // immediate
registry.QueueDestroyEntity(enemy);    // deferred to FlushQueues
```

Always queue from gameplay code. `QueueDestroyEntity` records the request and
`FlushQueues` runs it at the end of the frame, so iterating a component array never gets
invalidated mid-loop. Immediate destruction is for editor and tooling code.

`ClearEntities()` removes every entity and restarts ids at 1, while keeping component
types, systems, and resources registered. It is used when switching scenes.

## Components

Register before use:

```cpp
registry.RegisterComponent<Health>();
```

`Position`, `Rotation`, and `ScreenSpace` are registered by the registry itself; plugins
register the rest.

Add, read, and remove:

```cpp
registry.AddComponent(enemy, Engine::Health{ .current = 3, .max = 3 });

bool has = registry.HasComponent<Engine::Health>(enemy);
auto& health = registry.RequireComponent<Engine::Health>(enemy); // throws ErrorEcs if missing

std::optional<std::reference_wrapper<Engine::Health>> maybe =
    registry.GetComponent<Engine::Health>(enemy); // nullopt when absent

registry.RemoveComponent<Engine::Health>(enemy);
```

`AddComponent` inserts or replaces. The component array is a `std::unordered_map<Entity, T>`
per type, so iteration is cache-friendly relative to the number of entities of that type.

## Systems

Register in dependency order:

```cpp
auto system = registry.RegisterSystem<MovementSystem>(); // returns shared_ptr<T>
```

`RegisterSystem<T>()` returns a `std::shared_ptr<T>` so you can configure the system
afterwards. Registration order is update order and render order. Look up a registered
system with `GetSystem<T>()`.

`OnSceneUnloaded(Registry&)` is called on each system right before the scene's entities
are destroyed, so systems can drop state kept outside components.

## Resources

Global, one-per-game data:

```cpp
auto& assets = registry.AddResource<Engine::AssetManager>();
auto& input  = registry.GetResource<Engine::InputManager>(); // throws if missing
bool has     = registry.HasResource<Engine::InputManager>();
```

Use resources for things that are one per game (asset tables, input state, network host)
and components for things that are one per entity.

## Iteration

```cpp
for (auto& [entity, position] : *registry.GetComponentArray<Engine::Position>()) {
    position.x += 100.0f * dt;
}
```

`GetComponentArray<T>()` throws `ErrorEcs` if `T` was never registered. Checking several
components means a `HasComponent` per entity; there is no query language.

## Safety rules

- Keep components dumb: no methods, no owning pointers.
- Never destroy an entity directly while iterating; queue it.
- Register systems deliberately; the order is the update order.
- Bundle registration in a plugin when a feature needs a component, a system, and a resource.
