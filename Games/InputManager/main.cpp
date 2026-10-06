#include "Core/Application.hpp"
#include "Inputs/InputManager.hpp"
#include "Inputs/KeyboardController.hpp"
#include <iostream>
#include <raylib.h>

int main() {
    std::cout << "======================================\n";
    std::cout << "    InputManager Basic Test Demo      \n";
    std::cout << "======================================\n";

    // Application window configuration
    Engine::ApplicationConfig config;
    config.windowTitle = "InputManager Test Demo";
    config.windowWidth = 800;
    config.windowHeight = 450;
    config.targetFPS = 60;

    Engine::InputManager* inputPtr = nullptr;

    config.onUI = [&inputPtr]() {
        DrawText("INPUT MANAGER TEST", 30, 30, 24, RAYWHITE);
        DrawText("Press keys: WASD (Movement), SPACE, SHIFT, E, Q", 30, 70, 18, LIGHTGRAY);

        if (!inputPtr) {
            return;
        }
        auto* ctrl = inputPtr->GetController(1);
        if (!ctrl) {
            return;
        }

        const auto& actions = ctrl->getActionQueue();
        if (actions.empty()) {
            DrawText("No active action", 30, 130, 20, DARKGRAY);
        } else {
            int y = 130;
            for (auto action : actions) {
                const char* actionName = "UNKNOWN";
                switch (action) {
                    case Engine::ACTION_UP:
                        actionName = "ACTION_UP (W)";
                        break;
                    case Engine::ACTION_DOWN:
                        actionName = "ACTION_DOWN (S)";
                        break;
                    case Engine::ACTION_LEFT:
                        actionName = "ACTION_LEFT (A)";
                        break;
                    case Engine::ACTION_RIGHT:
                        actionName = "ACTION_RIGHT (D)";
                        break;
                    case Engine::ACTION_1:
                        actionName = "ACTION_1 (SPACE)";
                        break;
                    case Engine::ACTION_2:
                        actionName = "ACTION_2 (SHIFT)";
                        break;
                    case Engine::ACTION_3:
                        actionName = "ACTION_3 (E)";
                        break;
                    case Engine::ACTION_4:
                        actionName = "ACTION_4 (Q)";
                        break;
                    default:
                        break;
                }
                DrawText(TextFormat(">> Active Action: %s", actionName), 30, y, 20, GREEN);
                y += 30;
            }
        }
    };

    Engine::Application app(config);

    auto& inputManager = app.GetRegistry().AddResource<Engine::InputManager>();
    inputManager.AddPlayer(1, std::make_unique<Engine::KeyboardController>());
    inputPtr = &inputManager;

    app.Run();

    return 0;
}
