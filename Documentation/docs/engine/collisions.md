# Collision System

The Collision subsystem provides 2D collision detection and trigger management. It supports convex polygons, circles and rectangles
---

## Architecture & How It Works

The collision pipeline integrates ECS components, shape representations, and the SAT algorithm to detect contacts and
dispatch events each frame.

### 1. Shape Geometry & Representation

Shapes are defined via `Engine::ShapeComponent`:

* **`ShapeType::RECTANGLE`**: Generates 4 world-space corner vertices rotated around the rectangle's center.
* **`ShapeType::POLYGON`**: Generates $N$ regular polygon vertices spaced by angle steps around the center (e.g.
  `sides = 3` produces an equilateral triangle).
* **`ShapeType::CIRCLE`**: Approximated using a 24-sided regular polygon around the center. This allows circles to
  participate directly and efficiently in the SAT algorithm without separate math routines.

### 2. Solid Colliders vs. Trigger Zones

* **Solid Colliders (`isTrigger = false`)**: Represent physical solid barriers (e.g., player ships, walls, asteroids,
  enemy units). Fires `onCollisionEnter`, `onCollisionStay`, and `onCollisionExit`.
* **Trigger Zones (`isTrigger = true`)**: Represent non-blocking trigger areas (e.g., checkpoints, damage zones,
  pickups, level boundaries). Fires `onTriggerEnter`, `onTriggerStay`, and `onTriggerExit` (as well as collision
  events).

---

## Core Components & Plugin

### `Engine::Collider`

Attached to entities that need collision detection.

* `shape`: Optional embedded `ShapeComponent`. If not specified, the system looks for a `ShapeComponent` attached
  directly to the entity. If the collider has neither, it is skipped and the editor Console reports it once.
  That lookup does not throw when the entity has no shape.
* `enabled`: Set to `false` to disable collision checks without removing components.
* `isTrigger`: When `true`, designates the entity as a trigger zone.
* `isStatic`: When `true`, the collider does not move during resolution (wall / floor).
* `layer`: The single layer (0-31) this collider belongs to.
* `mask`: Bitmask of the layers this collider is willing to touch.

## Collision Layers & Masks

Two colliders only interact when **each one's mask contains the other's layer**, so either side can
veto a pair without touching the other entity:

| `layer` | `mask` | Meaning |
|---|---|---|
| 0 (`Default`) | `0xFFFFFFFF` | Default: touches everything, i.e. the behaviour before layers existed. |
| 1 (`Player`) | `0x10 \| 0x20` | Only touches `EnemyBullet` and `Wall`. |

The filter runs in `CollisionSystem::Update` **before** overlap detection, so a rejected pair skips
detection, physical resolution *and* the `onCollision*` / `onTrigger*` events. When a pair that was
in contact gets filtered out, the system reports the separation (`onCollisionExit` /
`onTriggerExit`) on the next frame.

Layers 0-7 have conventional names — `Default`, `Player`, `Enemy`, `PlayerBullet`, `EnemyBullet`,
`Wall`, `Pickup`, `Trigger` — and 8-31 are simply `Layer N`. The names are only a convention shared
by the Inspector and Lua; nothing enforces them. Helpers live in
`Engine/include/Components/CollisionLayers.hpp` (`Bit`, `Name`, `Index`, `kAllLayers`,
`kNoLayers`, `kDefaultLayer`).

In the editor, the Collider section of the Inspector shows a **Layer** combo and **Collides with**
checkboxes; the hex field reaches layers 8-31.

### Layers from Lua

```lua
Component.setColliderLayer(entity, "Player")                            -- index or name
Component.setColliderMask(entity, Layers.bit("EnemyBullet") + Layers.bit("Wall"))

-- Same predicate CollisionSystem uses: "would these two ever touch?"
if Component.collidersCanCollide(self.entity, other) then
    ...
end

log.info("layer " .. Component.getColliderLayerName(entity))
```

`Layers.name(i)`, `Layers.index("Enemy")`, `Layers.bit(i | "Enemy")`, `Layers.all`, `Layers.none`
and `Layers.count` are also available.

`Component.addBoxCollider(entity, w, h [, isTrigger [, isStatic]])` **reconfigures an existing
collider in place** rather than replacing it: the shape is always replaced, but layer, mask,
`enabled` and any flag that was not passed are left alone. Scripts typically call it from
`onCreate`, so clobbering those fields would silently discard the layer/mask authored in the scene
on the very first frame.

### `Engine::CollisionEvents`

Stores callbacks that trigger during the collision lifecycle:

* `onCollisionEnter`: Fired on the first frame two colliders touch.
* `onCollisionStay`: Fired every frame two colliders remain in contact.
* `onCollisionExit`: Fired on the frame two colliders separate.
* `onTriggerEnter`: Fired when an entity enters an overlapping trigger zone.
* `onTriggerStay`: Fired every frame an entity remains inside a trigger zone.
* `onTriggerExit`: Fired when an entity leaves a trigger zone.

### `Engine::CollisionPlugin`

Initializes the collision subsystem by registering `Position`, `Collider`, `CollisionEvents`, and `CollisionSystem` into
the registry.

---

## Step-by-Step Usage Guide (Based on the Sandbox Example)

The following walkthrough demonstrates how to build a playable collision sandbox with a **moving circle player**, a
**solid triangle obstacle**, and a **rectangular trigger zone** (from `Games/Collisions/main.cpp`).

### 1. Initialize Engine Plugins

```cpp
#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/CollisionPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/InputPlugin.hpp"

Engine::Application app(config);

// Register core plugins
Engine::PositionPlugin posPlugin;
posPlugin.Init(app.GetRegistry());

Engine::CollisionPlugin colPlugin;
colPlugin.Init(app.GetRegistry());

Engine::ShapePlugin shapePlugin;
shapePlugin.Init(app.GetRegistry());

Engine::InputPlugin inputPlugin;
inputPlugin.Init(app.GetRegistry());
```

### 2. Configure Player Controls

Setup `InputManager` to bind movement keys:

```cpp
auto& inputManager = app.GetRegistry().GetResource<Engine::InputManager>();
inputManager.AddPlayer(1, std::make_unique<Engine::Controller>());
auto* p1 = inputManager.GetController(1);

if (p1) {
    p1->addKeybind("MOVE_UP",    {KEY_W, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_UP",    {KEY_UP, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_DOWN",  {KEY_S, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_DOWN",  {KEY_DOWN, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_LEFT",  {KEY_A, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_LEFT",  {KEY_LEFT, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_RIGHT", {KEY_D, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_RIGHT", {KEY_RIGHT, 0, Engine::KEYBOARD});
}
```

### 3. Create the Moving Circle Player

```cpp
const auto player = app.GetRegistry().CreateEntity();
app.GetRegistry().AddComponent(player, Engine::Position{200.0f, 350.0f});

// Define Circle Shape
Engine::ShapeComponent playerShape;
playerShape.type = Engine::ShapeType::CIRCLE;
playerShape.radius = 40.0f;
playerShape.filled = true;
playerShape.color = GREEN;
app.GetRegistry().AddComponent(player, playerShape);

// Solid collider (enabled = true, isTrigger = false)
app.GetRegistry().AddComponent(player, Engine::Collider(true, false));

// Attach input controller link
Engine::InputPlugin::AttachToEntity(app.GetRegistry(), player, 1);

// Attach collision events
Engine::CollisionEvents playerEvents;
playerEvents.onCollisionEnter.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[PLAYER] Collided with Entity " << other << "!\n";
});
playerEvents.onCollisionExit.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[PLAYER] Separated from Entity " << other << "!\n";
});
playerEvents.onTriggerEnter.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[PLAYER] Entered Trigger Entity " << other << "!\n";
});
playerEvents.onTriggerExit.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[PLAYER] Exited Trigger Entity " << other << "!\n";
});
app.GetRegistry().AddComponent(player, playerEvents);
```

### 4. Create the Solid Triangle Obstacle

```cpp
const auto triangle = app.GetRegistry().CreateEntity();
app.GetRegistry().AddComponent(triangle, Engine::Position{550.0f, 350.0f});

// 3-sided polygon produces an equilateral triangle
Engine::ShapeComponent triangleShape;
triangleShape.type = Engine::ShapeType::POLYGON;
triangleShape.sides = 3;
triangleShape.radius = 70.0f;
triangleShape.filled = true;
triangleShape.color = GREEN;
app.GetRegistry().AddComponent(triangle, triangleShape);

// Solid collider
app.GetRegistry().AddComponent(triangle, Engine::Collider(true, false));

Engine::CollisionEvents solidEvents;
solidEvents.onCollisionEnter.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[SOLID TRIANGLE] Impact detected with Entity " << other << "!\n";
});
solidEvents.onCollisionExit.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[SOLID TRIANGLE] Contact ended with Entity " << other << "!\n";
});
app.GetRegistry().AddComponent(triangle, solidEvents);
```

### 5. Create the Rectangle Trigger Zone

```cpp
const auto triggerZone = app.GetRegistry().CreateEntity();
app.GetRegistry().AddComponent(triggerZone, Engine::Position{850.0f, 280.0f});

// Rectangle shape
Engine::ShapeComponent rectShape;
rectShape.type = Engine::ShapeType::RECTANGLE;
rectShape.width = 150.0f;
rectShape.height = 150.0f;
rectShape.filled = true;
rectShape.color = YELLOW;
app.GetRegistry().AddComponent(triggerZone, rectShape);

// Trigger collider (isTrigger = true)
app.GetRegistry().AddComponent(triggerZone, Engine::Collider(true, true));

Engine::CollisionEvents triggerEvents;
triggerEvents.onTriggerEnter.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[TRIGGER ENTER] Entity " << other << " stepped into Trigger Zone!\n";
});
triggerEvents.onTriggerExit.emplace_back([](Engine::Entity self, Engine::Entity other) {
    std::cout << "[TRIGGER EXIT] Entity " << other << " left Trigger Zone!\n";
});
app.GetRegistry().AddComponent(triggerZone, triggerEvents);
```

### 6. Querying Active Contacts Dynamically

You can also inspect active contact pairs directly from `Engine::CollisionSystem` without relying on event callbacks:

```cpp
auto collisionSystem = registry.GetSystem<Engine::CollisionSystem>();
const auto& contacts = collisionSystem->GetActiveContacts();

for (const auto& [entityA, entityB] : contacts) {
    // entityA and entityB are currently overlapping
}
```

This is how the example dynamically updates shape colors to `RED` when an overlap is active:

```cpp
class DynamicColorSystem : public Engine::System {
public:
    void Render(Engine::Registry& registry) override {
        auto collisionSystem = registry.GetSystem<Engine::CollisionSystem>();
        const auto& activeContacts = collisionSystem->GetActiveContacts();

        auto colliders = registry.GetComponentArray<Engine::Collider>();
        for (const auto& [entity, col] : *colliders) {
            if (!registry.HasComponent<Engine::ShapeComponent>(entity)) continue;

            const auto& shape = registry.GetComponent<Engine::ShapeComponent>(entity);

            bool isColliding = false;
            for (const auto& pair : activeContacts) {
                if (pair.first == entity || pair.second == entity) {
                    isColliding = true;
                    break;
                }
            }

            shape.color = isColliding ? RED : (col.isTrigger ? YELLOW : GREEN);
        }
    }
};
```
