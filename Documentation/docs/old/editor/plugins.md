# Editor plugins

Engine plugins make a component run. **Editor plugins** make it editable: they tell the
Inspector how to draw, add, and remove a component.

Without one, a registered component still shows up with a generic field editor. With one, you
control the labels, widgets, validation, and asset pickers.

---

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
} // namespace Editor
```

| Method | Purpose |
|---|---|
| `GetName()` | Section header in the Inspector |
| `GetIcon()` | Font Awesome icon shown next to the header |
| `OnImGuiRender()` | Draw the widgets; return `false` if the entity lacks the component |
| `HasComponent()` | Used to decide whether the section is shown |
| `AddComponent()` | Called when the user adds the component from the context menu |
| `RemoveComponent()` | Called when the user removes it |

---

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
        if (!HasComponent(entity, registry)) {
            return false;
        }

        auto& health = registry.GetComponent<Engine::Health>(entity);
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
} // namespace Editor
```

The header/source layout matches the existing plugins under
`Editor/include/Plugins/` and `Editor/src/Plugins/`.

---

## Registering it

The Inspector owns the list. Add one line to `InspectorPanel::InspectorPanel()` in
`Editor/src/Panels/InspectorPanel.cpp`:

```cpp
#include "Plugins/HealthPluginEditor.hpp"

InspectorPanel::InspectorPanel() {
    // ...
    m_PluginEditors.push_back(std::make_unique<HealthPluginEditor>());
}
```

Order in the vector is the order of sections inside the Inspector. Put `Name` and core
transform-like components first, then rendering, then gameplay, then networking.

---

## Guidelines

- **Do not duplicate engine defaults.** If the generic editor already shows the fields
  correctly, skip the plugin.
- **Use asset pickers for paths.** `SpritePluginEditor` shows the pattern for opening a file
  dialog and reloading the component.
- **Clamp early.** Drag widgets can emit values outside the valid range; clamp before the
  value reaches a system.
- **Keep serialization in sync.** If the inspector exposes a field, it must also be in
  `ComponentReflect.hpp` or the scene will not save it.
- **No logic here.** The editor only writes component data; systems do the work.

---

## See also

- [Interface](interface.md) — where these sections appear
- [Scene serialization](serialization.md) — reflection requirements
- [Plugins](../engine/plugins.md) — the runtime side
