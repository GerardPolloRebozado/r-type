# Examples

## Minimal engine plugin

```cpp
class HealthPlugin : public Engine::IPlugin {
  public:
    void Init(Engine::Registry& registry) override {
        registry.RegisterComponent<Health>();
        registry.RegisterSystem<HealthSystem>();
    }
};
```

## Minimal editor plugin

See [Editor plugins](editor-plugins.md) for a complete `HealthPluginEditor`.

## Shape example

The `Games/Shape` example registers `ShapePlugin` and draws a moving rectangle. The
`Games/Collisions` example adds colliders, layers, and trigger events.

## Networking example

The `Games/MultiplayerLowLevelSDK` example uses `Net.send` and `Net.on` directly. The
higher-level replication path is documented in [Networking](../engine/networking.md).

## See also

- [Engine plugins](engine-plugins.md)
- [Editor plugins](editor-plugins.md)
- [Lua plugins](lua.md)
