# Headless C++ Usage

Using the R-Type Engine without the editor, suitable for game projects, servers, and headset deployment.

## Initialization

The engine can be initialized in a headless mode by avoiding editor-specific components and systems.

```cpp
#include "Core/Application.hpp"
#include "Plugins/PositionPlugin.hpp"
#include "Plugins/RotationPlugin.hpp"
#include "Plugins/ShapePlugin.hpp"
#include "Plugins/SpritePlugin.hpp"
#include "Plugins/TextPlugin.hpp"
#include "Scripting/ScriptPlugin.hpp"

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "Headless Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.targetFPS = 60;
    config.maximized = false;

    Engine::Application app(config);

    // Initialize only the plugins needed for gameplay
    Engine::PositionPlugin positionPlugin;
    positionPlugin.Init(app.GetRegistry());
    Engine::SpritePlugin spritePlugin;
    spritePlugin.Init(app.GetRegistry());
    // ... other plugins

    app.Run();
    return 0;
}
```

## Scene Serialization

Scenes can be saved and loaded without the editor interface:

```cpp
#include "SceneSerializer.hpp"
#include "ECS/Registry.hpp"

Engine::Registry registry;
Engine::SceneSerializer serializer;
Engine::SceneConfig config;

// Serialize scene to JSON file
serializer.Serialize("scene.json", registry, config, Serialization::JSON);

// Load scene from JSON file
serializer.Deserialize("scene.json", registry, std::nullopt, Serialization::JSON);
```

## Headless Advantages

- No editor UI overhead
- Faster startup time
- Suitable for CI/CD pipelines
- Better suited for server-side game logic
- Reduced memory footprint (no editor resources loaded)