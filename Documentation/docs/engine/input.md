# Input Management

The Input Management subsystem provides a hardware-agnostic, dynamic multi-device input architecture for the R-Type Engine (inspired by Godot's `InputMap`). It decouples low-level physical input polling (Raylib keyboard, mouse buttons, gamepads, network packets) from the ECS simulation and gameplay logic.

---

## Architecture Overview

The input pipeline consists of four main layers:

```
[ Hardware Devices (Keyboard / Mouse / Gamepad) ]
                    ↓
[ IController / KeyboardController ]  ──> Translates hardware signals to string-based actions
                    ↓
[ InputManager (Global Resource) ]   ──> Polled once per frame during Application::PreUpdate
                    ↓
[ ECS Gameplay Systems ]             ──> Checks actions for entities with the Input component
```

1. **`Engine::DeviceType` & `Engine::Keybind`:** Identifies the device category (`KEYBOARD`, `MOUSE`, `GAMEPAD`, `TOUCH`) and binds a specific key code (plus optional combo modifier).
2. **`Engine::IController`:** Abstract interface representing a player's controller. Maintains a map of `std::string` actions to vectors of `Keybind`s (`m_keymap`) and updates `getActiveActions()` each tick.
3. **`Engine::KeyboardController`:** Multi-device controller implementation that polls Raylib keyboard, mouse, and gamepad APIs.
4. **`Engine::InputManager` (Resource):** A global ECS resource holding controller instances mapped to player IDs (`playerId`).
5. **`Engine::Input` (Component):** A lightweight ECS component tagging an entity with a `playerIdx` and an `isActive` toggle.
6. **`Engine::InputPlugin` (Plugin):** Helper plugin that registers the `Input` component and initializes the `InputManager` resource.

---

## 1. Setup & Dynamic Binding

Game developers can bind arbitrary custom actions to any combination of keyboard, mouse, or gamepad inputs:

```cpp
#include "Core/Application.hpp"
#include "Plugins/InputPlugin.hpp"
#include "Inputs/KeyboardController.hpp"

// Inside your initialization code:
Engine::InputPlugin inputPlugin;
inputPlugin.Init(app.GetRegistry());

auto& inputManager = app.GetRegistry().GetResource<Engine::InputManager>();
inputManager.AddPlayer(1, std::make_unique<Engine::KeyboardController>());
auto* p1 = inputManager.GetController(1);

// Dynamic runtime bindings for Player 1:
if (p1) {
    p1->addKeybind("MOVE_UP", {KEY_W, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_UP", {KEY_UP, 0, Engine::KEYBOARD}); // Alternate binding

    // Multi-device bindings: Shoot with Space, Left Mouse Click, or Gamepad A button
    p1->addKeybind("SHOOT", {KEY_SPACE, 0, Engine::KEYBOARD});
    p1->addKeybind("SHOOT", {MOUSE_BUTTON_LEFT, 0, Engine::MOUSE});
    p1->addKeybind("SHOOT", {GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0, Engine::GAMEPAD});
}
```

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

Gameplay systems query the `InputManager` by action name without needing to know which physical device triggered it:

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

            auto* controller = inputManager.GetController(input.playerIdx);
            if (!controller) {
                continue;
            }

            auto& pos = registry.GetComponent<Engine::Position>(entity);

            if (controller->isActionActive("MOVE_UP")) {
                pos.y -= 250.0f * dt;
            }
            if (controller->isActionActive("MOVE_DOWN")) {
                pos.y += 250.0f * dt;
            }
            if (controller->isActionActive("MOVE_LEFT")) {
                pos.x -= 250.0f * dt;
            }
            if (controller->isActionActive("MOVE_RIGHT")) {
                pos.x += 250.0f * dt;
            }
            if (controller->isActionActive("SHOOT")) {
                // Fire projectile logic...
            }
        }
    }
};
```

---

## 4. Multi-Player & Extensibility

- **Local Multiplayer:** Add additional players via `inputManager.AddPlayer(2, std::make_unique<Engine::KeyboardController>(1 /* gamepad index 1 */));`.
- **Custom Devices & Network:** Create custom controllers by subclassing `Engine::IController` and overriding `Update()`, `addKeybind()`, `isActionActive()`, and `getActiveActions()`. A `NetworkController` can populate `_activeActions` directly from incoming network packets.
