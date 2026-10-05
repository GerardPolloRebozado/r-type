# Text Rendering System

The Text system enables 2D text rendering in the R-Type Engine through the ECS architecture. It integrates with Raylib to support default bitmap fonts as well as custom TTF/OTF fonts with full control over size, character spacing, tint color, and world positioning.

---

## Architecture Overview

The system consists of three main elements:

1. **`Engine::Text` (Component):** A data component attached to entities holding text content, font reference, font size, spacing, and color.
2. **`Engine::TextSystem` (System):** A rendering system that iterates over all entities with a `Text` component and renders them at their `Position`.
3. **`Engine::TextPlugin` (Plugin):** A helper class to automatically register the component, system, and provide convenient entity attachment utilities.

---

## 1. Setup & Initialization

To enable text rendering in your game, initialize the `TextPlugin` with your ECS registry during startup:

```cpp
#include "Core/Application.hpp"
#include "Plugins/TextPlugin.hpp"

// Inside your initialization code:
Engine::TextPlugin textPlugin;
textPlugin.Init(app.GetRegistry());
```

`Init()` automatically registers:
- `Engine::Text` component
- `Engine::TextSystem` rendering system

---

## 2. Attaching Text to Entities

The `TextPlugin` provides static helper overloads to attach text to entities:

### Default Text (Default font size 10.0, White, Position 0,0)
```cpp
auto entity = registry.CreateEntity();
Engine::TextPlugin::AttachToEntity(registry, entity, "Hello World!");
```

### Specifying Font Size
```cpp
auto scoreEntity = registry.CreateEntity();
Engine::TextPlugin::AttachToEntity(registry, scoreEntity, "Score: 1000", 24.0f);
```

### Specifying Font Size and Position
```cpp
auto titleEntity = registry.CreateEntity();
Engine::TextPlugin::AttachToEntity(registry, titleEntity, "R-TYPE", 32.0f, Engine::Position{400.0f, 100.0f});
```

> **Note:** If an entity already has an existing `Position` component, `AttachToEntity` preserves the existing position rather than overwriting it.

---

## 3. Custom Fonts, Colors, and Spacing

You can configure the `Text` component directly for advanced styling:

```cpp
#include "Components/Text.hpp"
#include "Components/Position.hpp"

auto entity = registry.CreateEntity();

// Load custom Raylib font
Font customFont = LoadFont("assets/fonts/custom.ttf");

// Add Text component with custom font, size, spacing, and color
registry.AddComponent(entity, Engine::Text{
    customFont,        // Custom font
    "Game Over",       // Text content
    48.0f,             // Font size in pixels
    2.0f,              // Character spacing
    RED                // Color tint
});

registry.AddComponent(entity, Engine::Position{350.0f, 250.0f});
```

---

## 4. Error Handling & Robustness

- **Missing Position Protection:** If an entity has a `Text` component but lacks a `Position` component, `TextSystem::Update` safely skips rendering without crashing or throwing `std::out_of_range`.
- **Default Font Fallback:** If `font.texture.id == 0` (no custom font loaded), the system automatically falls back to Raylib's built-in `DrawText` font.
