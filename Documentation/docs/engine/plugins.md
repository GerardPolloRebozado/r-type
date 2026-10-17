# Plugins

Plugins bundle components and systems together.

To create a plugin, implement the `IPlugin` interface:
```cpp
class MyPlugin : public Engine::IPlugin {
public:
    void Init(Engine::Registry& registry) override {
        registry.RegisterComponent<MyComponent>();
        registry.RegisterSystem<MySystem>();
    }
};
```

You can also generate the files from the terminal. See [Command line](../editor/cli.md).
