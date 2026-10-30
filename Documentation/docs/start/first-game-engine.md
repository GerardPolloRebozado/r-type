# Your first game in C++

This is the shortest path from an empty `main.cpp` to something moving on screen. It takes
about ten minutes and introduces the four pieces every game has: an application, a plugin, an
entity, and a system.

## 1. The application

Create `main.cpp`:

```cpp
#include "Core/Application.hpp"

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "My First Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    app.Run();
    return 0;
}
```

`Application` owns the window, the registry, and the loop. `Run()` blocks until the window
closes. At this point you have a black window: nothing is registered and nothing is drawn.

## 2. A plugin

Plugins register components, systems, and resources. Add the shape plugin so we can draw a
rectangle without loading an asset:

```cpp
#include "Plugins/ShapePlugin.hpp"

Engine::ShapePlugin shapes;
shapes.Init(app.GetRegistry());
```

`ShapePlugin::Init()` registers `ShapeComponent` and `ShapeSystem`, along with the `Position`
component and the opacity component. Registration order inside the plugin is already correct.

## 3. An entity

```cpp
Engine::Registry& registry = app.GetRegistry();
Engine::Entity player = registry.CreateEntity();

Engine::ShapeComponent body;
body.type = Engine::ShapeType::RECTANGLE;
body.width = 48.0f;
body.height = 32.0f;
body.color = SKYBLUE;

Engine::ShapePlugin::AttachToEntity(registry, player, body);
registry.RequireComponent<Engine::Position>(player) =
    Engine::Position(220.0f, 320.0f, 10);
```

`CreateEntity()` gives every entity a default `Position` and `Rotation`. `RequireComponent<T>()`
returns a reference and throws `ErrorEcs` if the component is missing; `GetComponent<T>()`
returns an `std::optional` when you want to check first.

Run it: a blue rectangle appears at `(220, 320)`. It is drawn by `ShapeSystem`, which looks for
entities with a `Position` and a `ShapeComponent`.

## 4. A system

Static rectangles are not a game. A system moves things:

```cpp
#include "Components/Position.hpp"
#include "ECS/System.hpp"

class MoveSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        for (auto& [entity, position] : *registry.GetComponentArray<Engine::Position>()) {
            position.x += 140.0f * dt;

            if (position.x > 1200.0f) {
                position.x = -48.0f; // wrap around
            }
        }
    }
};
```

Register it after the shape plugin:

```cpp
registry.RegisterSystem<MoveSystem>();
```

Systems update in registration order, once per fixed step. `dt` is the step length in seconds,
so the speed is the same at 30, 60, or 144 FPS.

## 5. The complete `main.cpp`

```cpp
#include "Components/Position.hpp"
#include "Components/ShapeComponent.hpp"
#include "Core/Application.hpp"
#include "ECS/System.hpp"
#include "Plugins/ShapePlugin.hpp"

class MoveSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        for (auto& [entity, position] : *registry.GetComponentArray<Engine::Position>()) {
            position.x += 140.0f * dt;
            if (position.x > 1200.0f) {
                position.x = -48.0f;
            }
        }
    }
};

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "My First Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    Engine::Registry& registry = app.GetRegistry();

    Engine::ShapePlugin shapes;
    shapes.Init(registry);

    Engine::Entity player = registry.CreateEntity();

    Engine::ShapeComponent body;
    body.type = Engine::ShapeType::RECTANGLE;
    body.width = 48.0f;
    body.height = 32.0f;
    body.color = SKYBLUE;
    Engine::ShapePlugin::AttachToEntity(registry, player, body);
    registry.RequireComponent<Engine::Position>(player) =
        Engine::Position(220.0f, 320.0f, 10);

    registry.RegisterSystem<MoveSystem>();

    app.Run();
    return 0;
}
```

Build it as part of the project, or add it to `Games/CMakeLists.txt`, run it, and the rectangle
crosses the window.

## Where to go next

| I want to… | Read |
|---|---|
| Understand the model | [Core concepts](concepts.md) and [Registry](../engine/registry.md) |
| Add sprites and text | [Rendering](../engine/rendering.md) |
| React to the keyboard | [Input](../engine/input.md) |
| Play sounds | [Audio](../engine/audio.md) |
| Detect hits and triggers | [Collisions](../engine/collisions.md) |
| Build and edit scenes visually | [Interface](../editor/interface.md) |
