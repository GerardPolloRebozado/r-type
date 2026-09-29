#include "Core/Application.hpp"
#include <iostream>
#include <imgui.h>
#include <rlImGui.h>

int main() {
    std::cout << "======================================\n";
    std::cout << "      R-Type Editor - Hello World     \n";
    std::cout << "======================================\n";

    Engine::ApplicationConfig config;
    config.windowTitle = "R-Type Editor";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    
    // UI Hook that the Engine calls between BeginDrawing() and EndDrawing()
    config.onUI = []() {
        rlImGuiBegin();

        // Basic UI Demo Setup
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::Begin("Inspector")) {
            ImGui::Text("Hello from the Editor!");
            if (ImGui::Button("Click Me!")) {
                std::cout << "Button clicked in Inspector!\n";
            }
        }
        ImGui::End();

        if (ImGui::Begin("Scene Hierarchy")) {
            ImGui::Text("Entities will appear here.");
        }
        ImGui::End();

        rlImGuiEnd();
    };

    Engine::Application app(config);

    // Initialize ImGui integration after Raylib window is created
    rlImGuiSetup(true);

    // Run the main blocking game loop
    app.Run();

    // Cleanup ImGui
    rlImGuiShutdown();

    return 0;
}
