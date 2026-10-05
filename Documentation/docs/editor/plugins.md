# Editor Plugins

To expose a component to the Inspector Panel, you must write an `IPluginEditor`.

```cpp
class TransformPluginEditor : public Editor::IPluginEditor {
public:
    void OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override {
        // Draw ImGui controls here
    }
};
```
