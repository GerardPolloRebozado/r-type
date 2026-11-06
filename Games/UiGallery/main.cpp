#include "Assets/ProjectPaths.hpp"
#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include "UI/UiComponent.hpp"
#include "UI/UiPlugin.hpp"
#include <string>

#ifndef UI_GALLERY_SOURCE_DIR
#define UI_GALLERY_SOURCE_DIR "."
#endif

int main() {
    Engine::ApplicationConfig config;
    Engine::Entity gallery = 0;

    config.windowTitle = "UI Gallery";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;

    Engine::Application app(config);
    auto& registry = app.GetRegistry();

    registry.AddResource<Engine::ProjectContext>(std::string(UI_GALLERY_SOURCE_DIR));
    // CreateEntity always adds Position and Rotation, so those plugins have to be present.
    Engine::PositionPlugin().Init(registry);
    Engine::RotationPlugin().Init(registry);
    Engine::ScriptPlugin().Init(registry);
    Engine::UiPlugin().Init(registry);

    gallery = registry.CreateEntity();
    Engine::UiPlugin::AttachToEntity(registry, gallery, Engine::UiComponent("ui/gallery.lua"));

    app.Run();
    return 0;
}
