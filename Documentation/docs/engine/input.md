# Input

The input system is hardware-agnostic and dynamic. You bind string actions to physical
keys, mouse buttons, or gamepad buttons at runtime, and gameplay reads actions by name.

## Setup

```cpp
Engine::InputPlugin inputPlugin;
inputPlugin.Init(app.GetRegistry());

auto& inputManager = app.GetRegistry().GetResource<Engine::InputManager>();
inputManager.AddPlayer(1, std::make_unique<Engine::Controller>());
auto* p1 = inputManager.GetController(1);

if (p1) {
    p1->addKeybind("MOVE_UP", {KEY_W, 0, Engine::KEYBOARD});
    p1->addKeybind("MOVE_UP", {KEY_UP, 0, Engine::KEYBOARD});
    p1->addKeybind("SHOOT", {KEY_SPACE, 0, Engine::KEYBOARD});
    p1->addKeybind("SHOOT", {MOUSE_BUTTON_LEFT, 0, Engine::MOUSE});
    p1->addKeybind("SHOOT", {GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0, Engine::GAMEPAD});
}
```

`Controller` polls Raylib keyboard, mouse, and gamepad APIs. Subclass `IController` to feed
actions from a network packet or a custom device.

## Attaching input to entities

```cpp
Engine::Entity player = registry.CreateEntity();
Engine::InputPlugin::AttachToEntity(registry, player, 1);
```

The `Input` component has `playerIdx` and `isActive`. Set `isActive = false` to ignore
input for an entity during cutscenes or stuns.

## Reading input in a system

```cpp
class MovementSystem : public Engine::System {
  public:
    void Update(Engine::Registry& registry, float dt) override {
        if (!registry.HasResource<Engine::InputManager>()) return;
        auto& inputManager = registry.GetResource<Engine::InputManager>();

        for (auto& [entity, input] : *registry.GetComponentArray<Engine::Input>()) {
            if (!input.isActive) continue;
            auto* controller = inputManager.GetController(input.playerIdx);
            if (!controller) continue;

            auto& pos = registry.RequireComponent<Engine::Position>(entity);
            if (controller->isActionActive("MOVE_UP"))    pos.y -= 250.0f * dt;
            if (controller->isActionActive("MOVE_DOWN"))   pos.y += 250.0f * dt;
            if (controller->isActionActive("MOVE_LEFT"))   pos.x -= 250.0f * dt;
            if (controller->isActionActive("MOVE_RIGHT"))  pos.x += 250.0f * dt;
        }
    }
};
```

## Multi-player

Add more players with `inputManager.AddPlayer(2, std::make_unique<Engine::Controller>(1))`.
In multiplayer, the client sends input to the server; the server simulates. See
[Networking](networking.md).

## Input mapping in the editor

The editor stores actions and keybinds in `input.json` and provides the **Input Map** panel
for editing them without code. The map is baked into the game launcher, so changing it
requires a rebuild.
