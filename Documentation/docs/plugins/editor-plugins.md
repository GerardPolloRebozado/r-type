# Editor plugins

Engine plugins make a component run. **Editor plugins** make it editable: they tell the
Inspector how to draw, add, and remove a component.

Without one, a registered component still shows up with a generic field editor. With one,
you control labels, widgets, validation, and asset pickers.

## The `IPluginEditor` interface

```cpp
#include "PluginEditor.hpp"

namespace Editor {
class IPluginEditor {
  public:
    virtual ~IPluginEditor() = default;
    virtual const char* GetName() const = 0;
    virtual const char* GetIcon() const = 0;
    virtual bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) = 0;
    virtual bool HasComponent(Engine::Entity entity, Engine::Registry& registry) = 0;
    virtual void AddComponent(Engine::Entity entity, Engine::Registry& registry) = 0;
    virtual void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) = 0;
};
}
```

## A minimal editor plugin

```cpp
#include "Components/Health.hpp"
#include "PluginEditor.hpp"
#include <imgui.h>

namespace Editor {
class HealthPluginEditor : public IPluginEditor {
  public:
    const char* GetName() const override { return "Health"; }
    const char* GetIcon() const override { return ICON_FA_HEART; }

    bool HasComponent(Engine::Entity entity, Engine::Registry& registry) override {
        return registry.HasComponent<Engine::Health>(entity);
    }

    bool OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) override {
        if (!HasComponent(entity, registry)) return false;
        auto& health = registry.RequireComponent<Engine::Health>(entity);
        ImGui::DragInt("Current", &health.current, 1.0f, 0, health.max);
        ImGui::DragInt("Max", &health.max, 1.0f, 1, 9999);
        ImGui::Checkbox("Invincible", &health.invincible);
        return true;
    }

    void AddComponent(Engine::Entity entity, Engine::Registry& registry) override {
        registry.AddComponent(entity, Engine::Health{});
    }

    void RemoveComponent(Engine::Entity entity, Engine::Registry& registry) override {
        registry.RemoveComponent<Engine::Health>(entity);
    }
};
}
```

## Registering it

Add one line to `InspectorPanel::InspectorPanel()` in
`Editor/src/Panels/InspectorPanel.cpp`:

```cpp
#include "Plugins/HealthPluginEditor.hpp"
m_PluginEditors.push_back(std::make_unique<HealthPluginEditor>());
```

Order in the vector is the order of sections in the Inspector.

## Guidelines

- Do not duplicate engine defaults. If the generic editor already shows the fields
  correctly, skip the plugin.
- Use asset pickers for paths.
- Clamp early; drag widgets can emit out-of-range values.
- Keep serialization in sync: every field the inspector exposes must also be in
  `ComponentReflect.hpp`.
- No logic here. The editor only writes component data; systems do the work.

## See also

- [Interface](../editor/interface.md)
- [Scene format](../editor/scenes.md)
- [Engine plugins](engine-plugins.md)
- [Adding editor features](../contributing/adding-editor-features.md)
