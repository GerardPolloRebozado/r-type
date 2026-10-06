# Input Management

The Input Management subsystem provides a hardware-agnostic, multi-device input architecture for the R-Type Engine. It decouples low-level physical input polling (Raylib keyboard, gamepads, network packets) from the ECS simulation and gameplay logic.

---

## Architecture Overview

The input pipeline consists of four main layers:

```
[ Hardware Devices (Keyboard / Gamepad) ]
                    ↓
[ IController / KeyboardController ]  ──> Translates hardware keys to ActionType enums
                    ↓
[ InputManager (Global Resource) ]   ──> Polled once per frame during Application::PreUpdate
                    ↓
[ ECS Gameplay Systems ]             ──> Reads active actions for entities with Input component
```

1. **`Engine::ActionType`:** An enum representing abstract gameplay actions (`ACTION_UP`, `ACTION_DOWN`, `ACTION_LEFT`, `ACTION_RIGHT`, `ACTION_1`, etc.).
2. **`Engine::IController`:** Abstract interface representing a physical or virtual input device. Concrete implementations like `KeyboardController` poll hardware and maintain an active action queue.
3. **`Engine::InputManager` (Resource):** A global ECS resource holding controller instances mapped to player IDs (`playerId`).
4. **`Engine::Input` (Component):** A lightweight ECS component that tags an entity with a `playerIdx` and an `isActive` toggle.
5. **`Engine::InputPlugin` (Plugin):** Helper plugin that registers the `Input` component and initializes the `InputManager` resource.

---

## 1. Setup & Initialization

To enable input management, initialize the `InputPlugin` and configure player controllers at startup:

```cpp
#include "Core/Application.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Inputs/KeyboardController.hpp"

// Inside your initialization code:
Engine::InputPlugin inputPlugin;
inputPlugin.Init(app.GetRegistry());

// Fetch the InputManager resource and bind Player 1 to the keyboard
auto& inputManager = app.GetRegistry().GetResource<Engine::InputManager>();
inputManager.AddPlayer(1, std::make_unique<Engine::KeyboardController>());
```

`InputPlugin::Init()` automatically registers:
- `Engine::Input` component.
- `Engine::InputManager` resource (if not already added).

---

## 2. Attaching Input to Entities

Use `InputPlugin::AttachToEntity` to link an entity to a player ID:

```cpp
// Create a player entity
Engine::Entity player = registry.CreateEntity();

// Attach the Input component to link this entity to Player 1
Engine::InputPlugin::AttachToEntity(registry, player, 1);
```

The `Input` component has two fields:
* `playerIdx`: Identifier of the player controlling the entity.
* `isActive`: Boolean indicating whether the entity is currently responsive to input (useful for unit selection, cutscenes, or temporary stuns).

---

## 3. Reading Input in Gameplay Systems

Gameplay systems should read from the `InputManager` resource and iterate over entities with the `Input` component:

```cpp
#include "ECS/System.hpp"
#include "Components/Input.hpp"
#include "Components/Position.hpp"
#include "Inputs/InputManager.hpp"

class MovementSystem : public Engine::System {
public:
    void Update(Engine::Registry& registry, float dt) override {
        if (!registry.HasResource<Engine::InputManager>()) {
            return;
        }

        auto& inputManager = registry.GetResource<Engine::InputManager>();

        for (Engine::Entity entity : m_Entities) {
            auto& input = registry.GetComponent<Engine::Input>(entity);
            if (!input.isActive) {
                continue; // Skip inactive entities
            }

            Engine::IController* controller = inputManager.GetController(input.playerIdx);
            if (!controller) {
                continue;
            }

            auto& pos = registry.GetComponent<Engine::Position>(entity);
            const auto& actions = controller->getActionQueue();

            for (Engine::ActionType action : actions) {
                switch (action) {
                    case Engine::ACTION_UP:    pos.y -= 250.0f * dt; break;
                    case Engine::ACTION_DOWN:  pos.y += 250.0f * dt; break;
                    case Engine::ACTION_LEFT:  pos.x -= 250.0f * dt; break;
                    case Engine::ACTION_RIGHT: pos.x += 250.0f * dt; break;
                    default: break;
                }
            }
        }
    }
};
```

---

## 4. Default Keyboard Mappings

The `KeyboardController` provides standard default bindings:

| Action | Default Key |
| :--- | :--- |
| `ACTION_UP` | `W` |
| `ACTION_DOWN` | `S` |
| `ACTION_LEFT` | `A` |
| `ACTION_RIGHT` | `D` |
| `ACTION_1` | `Space` |
| `ACTION_2` | `Left Shift` |
| `ACTION_3` | `E` |
| `ACTION_4` | `Q` |

---

## 5. Multi-Player & Extensibility

- **Local Multiplayer:** Add additional players by calling `inputManager.AddPlayer(2, std::make_unique<GamepadController>(0));`. Entities with `playerIdx = 2` will automatically read from Gamepad 0.
- **Custom Devices & Network:** Create custom controllers by subclassing `Engine::IController` and overriding `poll()`, `Update()`, `setDefaultKeymap()`, and `getActionQueue()`. For network multiplayer, a `NetworkController` can populate the action queue directly from incoming network packets.
