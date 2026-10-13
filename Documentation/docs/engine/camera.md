# Camera System

The Camera system decides which part of the world is shown on screen. It lets a game have levels bigger than the window, zoom and rotate the view, follow the player smoothly, stay inside the level limits and shake on impacts, while HUD elements stay fixed.

---

## Architecture Overview

1. **`Engine::CameraComponent` (Component):** Camera settings: zoom, primary flag, follow target, bounds and shake.
2. **`Engine::CameraSystem` (System):** Moves cameras towards their follow target, fades the shake and builds the raylib `Camera2D` used for rendering.
3. **`Engine::CameraPlugin` (Plugin):** Registers the component and the system, and attaches cameras to entities.
4. **`Engine::ScreenSpace` (Tag component):** Marks entities drawn in window pixels, ignoring the camera (score, lives, menus). Always registered by the `Registry`.

A camera is a normal entity and reuses its transform:

| Entity component | Meaning for the camera |
|---|---|
| `Position` | The world point shown at the **center** of the screen |
| `Rotation` | The rotation of the view, in degrees |

Moving the camera means moving its entity, the same way as any other entity (code, editor gizmo, hierarchy).

### Render passes

Every frame is drawn in two passes by `Engine::RenderScene`, used by both the game and the Editor scene view:

1. **World pass:** each system's `Render()`, inside `BeginMode2D(activeCamera)`.
2. **Screen pass:** each system's `RenderScreen()`, with no camera.

`SpriteSystem`, `TextSystem` and `ShapeSystem` draw entities tagged `ScreenSpace` in the screen pass and everything else in the world pass. When writing your own render system, override `Render` for world drawing and `RenderScreen` for HUD drawing.

> **Without a camera nothing changes:** if no primary camera exists, the world pass uses an identity camera, so world coordinates are window pixels exactly as before.

---

## 1. Setup & Initialization

```cpp
#include "Plugins/CameraPlugin.hpp"

Engine::CameraPlugin cameraPlugin;
cameraPlugin.Init(app.GetRegistry());
```

`Init()` registers:
- `Engine::CameraComponent`
- `Engine::CameraSystem`

> **Tip:** systems update in registration order. Initialise `CameraPlugin` after the systems that move the player, so the camera follows the movement of the same frame.

## 2. Adding a camera

```cpp
auto camera = registry.CreateEntity();
Engine::CameraPlugin::AttachToEntity(registry, camera, Engine::CameraComponent{});
```

`AttachToEntity` keeps the entity's existing `Position`, or adds one if it has none.

### Following an entity

```cpp
Engine::CameraComponent cam;
cam.followTarget = player;  // entity to follow
cam.followSpeed = 5.0f;     // 0 = snap instantly, higher = catches up faster
Engine::CameraPlugin::AttachToEntity(registry, camera, cam);
```

Alternative: make the camera a child of the player with the Hierarchy plugin (`Position::relative = true`). It then moves rigidly with the player, without smoothing.

### Keeping the view inside the level

```cpp
cam.clampToBounds = true;
cam.bounds = {0.0f, 0.0f, levelWidth, levelHeight};
```

The view never shows anything outside `bounds`. If the level is smaller than the view on one axis, the camera centers on it. The clamp is applied when rendering and does not change the camera's `Position`.

> The clamp ignores rotation: a rotated camera can show a little outside the bounds in its corners.

### Zoom, rotation and shake

```cpp
auto& cam = registry.GetComponent<Engine::CameraComponent>(camera);
cam.zoom = 2.0f;                                               // everything twice as big
registry.GetComponent<Engine::Rotation>(camera).angle = 10.0f; // degrees
cam.shakeTrauma = 1.0f;                                        // full shake
```

Shake strength is `shakeMagnitude * trauma²` and trauma decreases by `shakeDecay` per second, so small hits barely shake and big hits shake a lot. Shake is only visual: it never moves the camera's `Position`.

### Several cameras

Only one camera renders: the one with `primary = true` and the lowest entity id. Switch cameras by toggling `primary`.

---

## 3. HUD elements

```cpp
auto score = registry.CreateEntity();
Engine::TextPlugin::AttachToEntity(registry, score, "Score: 0", 24.0f);
registry.GetComponent<Engine::Position>(score) = Engine::Position{10.0f, 10.0f};
registry.AddComponent(score, Engine::ScreenSpace{});
```

The position of a `ScreenSpace` entity is in window pixels.

---

## 4. Mouse and coordinates

The mouse is in window pixels. Convert it before comparing it with world positions:

```cpp
Vector2 world = Engine::CameraSystem::ScreenToWorld(registry, GetMousePosition());
Vector2 screen = Engine::CameraSystem::WorldToScreen(registry, {pos.x, pos.y});
```

Collisions and all gameplay use world coordinates and are not affected by the camera.

---

## 5. Editor

- Right-click in the Inspector and choose **Add Camera**.
- Every setting is editable. **Test Shake** previews the shake.
- Position and rotation are edited with the entity's Position and Rotation sections or the scene gizmo.
- The camera is saved in the scene (`"CameraComponent"`), and built games initialise `CameraPlugin` automatically.
- The Scene view renders through the active camera, and the gizmo takes it into account.

> **Follow target** is stored as an entity id. Load scenes into an empty registry so the ids match.

---

## 6. Multiplayer

The camera is client-side only. Do not replicate it: each client follows its own player locally.

---

## Example

See `Games/Camera`: a world larger than the window, a camera following the player with smoothing and bounds, zoom, rotation, shake, a HUD, and mouse clicks converted to world positions.
