# Headless C++ Usage API Reference

This section provides the C++ API reference for headless engine usage. It documents the core classes and systems available when running the engine without the editor.

## Core Classes

### `Engine::Application`

The main application class that manages the game loop, window, and resources.

**Header:** `#include "Core/Application.hpp"`

**Key Methods:**
- `Application::Application(const ApplicationConfig& config)` - Constructor
- `Application::Run()` - Starts the main loop
- `Application::GetRegistry()` - Returns the ECS registry
- `Application::GetResource<T>()` - Gets a resource from the registry

### `Engine::Registry`

The ECS registry that manages entities, components, and systems.

**Header:** `#include "ECS/Registry.hpp"`

**Key Methods:**
- `Registry::CreateEntity()` - Creates a new entity
- `Registry::HasComponent<T>(Entity)` - Checks if an entity has a component
- `Registry::AddComponent<T>(Entity, T)` - Adds a component to an entity
- `Registry::RemoveComponent<T>(Entity)` - Removes a component from an entity
- `Registry::GetComponent<T>(Entity)` - Gets a component from an entity

### `Engine::Position`, `Engine::Rotation`, `Engine::SpriteComponent`, etc.

Standard ECS components used in headless mode.

### `Engine::ScriptSystem`

Manages Lua script loading and execution.

**Header:** `#include "Scripting/ScriptSystem.hpp"`

**Key Methods:**
- `ScriptSystem::LoadScript(Entity, const std::string&)` - Loads a Lua script file
- `ScriptSystem::UnloadScript(Entity)` - Unloads a Lua script

## Plugin System

Plugins are used to register components and systems without the editor.

**Header:** `#include "Plugins/IPlugin.hpp"

**Example Plugin:**

```cpp
class MyPlugin : public IPlugin {
public:
    void Init(Registry& registry) override {
        registry.RegisterComponent<Position>();
        registry.RegisterSystem<MySystem>();
    }
};
```

## Scene Serialization (Headless)

Scenes can be saved and loaded without the editor interface.

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