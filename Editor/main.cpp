#include "Core/Application.hpp"
#include "Editor/EditorLayer.hpp"
#include <iostream>

int main() {
    std::cout << "======================================\n";
    std::cout << "      R-Type Editor - Hello World     \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "R-Type Editor";
    config.windowWidth = 1280;
    config.windowHeight = 720;

    Editor::EditorLayer editorLayer;

    config.onUI = [&editorLayer]() { editorLayer.OnUpdate(); };

    Engine::Application app(config);

    editorLayer.Init();

    app.Run();

    editorLayer.Shutdown();

    return 0;
}
