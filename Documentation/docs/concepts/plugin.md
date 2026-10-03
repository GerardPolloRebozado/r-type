# Plugins

A **Plugin** is a clean way to bundle related Components, Systems, and Resources together and inject them into the engine with a single line of code.

## Why Plugins?

If you create an `AudioSource` component and an `AudioSystem`, you need to register both of them into the `Registry`. Furthermore, you need to tell the `Registry` that the `AudioSystem` only cares about entities that have an `AudioSource`.

Instead of cluttering your main game loop with registration logic, you encapsulate it inside a Plugin.

## Creating a Plugin

Inherit from `Engine::IPlugin` and implement the `Init()` method.

```cpp
#include "Plugins/IPlugin.hpp"

class AudioPlugin : public Engine::IPlugin {
public:
    void Init(Engine::Registry& registry) override {
        // 1. Register the Resource
        if (!registry.HasResource<AssetManager>()) {
            registry.EmplaceResource<AssetManager>();
        }

        // 2. Register the Component
        registry.RegisterComponent<AudioSource>();

        // 3. Register the System
        auto system = registry.RegisterSystem<AudioSystem>();

        // 4. Define the Signature (What components does this system need?)
        Engine::Signature signature;
        signature.set(registry.GetComponentType<AudioSource>());
        registry.SetSystemSignature<AudioSystem>(signature);
    }
};
```

## Using a Plugin

In your application setup, simply instantiate and initialize the plugin:

```cpp
Engine::AudioPlugin audioPlugin;
audioPlugin.Init(app.GetRegistry());
```
