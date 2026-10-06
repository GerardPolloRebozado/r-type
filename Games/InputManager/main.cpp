#include "Core/Application.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/InputManager.hpp"
#include <iostream>
#include <raylib.h>

int main() {
    std::cout << "======================================\n";
    std::cout << "    InputManager Dynamic Test Demo    \n";
    std::cout << "======================================\n";

    // Application window configuration
    Engine::ApplicationConfig config;
    config.windowTitle = "InputManager Dynamic InputMap Demo";
    config.windowWidth = 800;
    config.windowHeight = 450;
    config.targetFPS = 60;

    Engine::InputManager* inputPtr = nullptr;

    config.onUI = [&inputPtr]() {
        DrawText("DYNAMIC INPUTMAP TEST", 30, 30, 24, RAYWHITE);
        DrawText("Keys: WASD / Arrows, SPACE / Left Click / Gamepad A (Shoot), SHIFT / Right Click "
                 "(Boost)",
                 30, 70, 16, LIGHTGRAY);

        if (!inputPtr) {
            return;
        }
        auto* ctrl = inputPtr->GetController(1);
        if (!ctrl) {
            return;
        }

        const auto& actions = ctrl->getActiveActions();
        if (actions.empty()) {
            DrawText("No active action", 30, 130, 20, DARKGRAY);
        } else {
            int y = 130;
            for (const auto& action : actions) {
                DrawText(TextFormat(">> Active Action: [%s]", action.c_str()), 30, y, 20, GREEN);
                y += 30;
            }
        }
    };

    Engine::Application app(config);

    // Register InputManager as an engine resource and attach a controller to Player 1
    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    inputManager.AddPlayer(1, std::make_unique<Engine::Controller>());
    inputPtr = &inputManager;

    auto* p1 = inputManager.GetController(1);

    // Game Developer binds custom actions dynamically on the player's controller:
    if (p1) {
        p1->addKeybind("MOVE_UP", {KEY_W, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_UP", {KEY_UP, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_DOWN", {KEY_S, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_DOWN", {KEY_DOWN, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_LEFT", {KEY_A, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_LEFT", {KEY_LEFT, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_RIGHT", {KEY_D, 0, Engine::KEYBOARD});
        p1->addKeybind("MOVE_RIGHT", {KEY_RIGHT, 0, Engine::KEYBOARD});

        // Multi-device binding: Shoot via Space (Keyboard), Left-Click (Mouse), or Button A
        // (Gamepad)
        p1->addKeybind("SHOOT", {KEY_SPACE, 0, Engine::KEYBOARD});
        p1->addKeybind("SHOOT", {MOUSE_BUTTON_LEFT, 0, Engine::MOUSE});
        p1->addKeybind("SHOOT", {GAMEPAD_BUTTON_RIGHT_FACE_DOWN, 0, Engine::GAMEPAD});

        // Boost via Left Shift (Keyboard) or Right-Click (Mouse)
        p1->addKeybind("BOOST", {KEY_LEFT_SHIFT, 0, Engine::KEYBOARD});
        p1->addKeybind("BOOST", {MOUSE_BUTTON_RIGHT, 0, Engine::MOUSE});
    }

    // Start engine loop
    app.Run();

    return 0;
}
