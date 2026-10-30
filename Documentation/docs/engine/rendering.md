# Rendering

The renderer draws the registry in two passes every frame: a **world pass** inside the
active camera, then a **screen pass** in window pixels for HUD elements. Entities tagged
`ScreenSpace` are drawn in the screen pass; everything else in the world pass.

Within each pass, layered systems (sprites, shapes, text) draw first, sorted by
`Position::z`, then by system registration order, then by entity id. Non-layered systems
then run their `Render` / `RenderScreen` in registration order.

## Sprites

`SpriteComponent` names an asset and places it relative to the entity:

```cpp
Engine::SpriteComponent sprite;
sprite.assetPath = "assets/player.png";
sprite.offsetX = -12.8f;
sprite.offsetY = -14.0f;
sprite.scaleX = 4.0f;
sprite.scaleY = 4.0f;
sprite.flipX = false;
sprite.flipY = false;
```

The sprite's top-left corner is drawn at `(Position.x + offsetX, Position.y + offsetY)`.
Positions are top-left corners everywhere in the engine, which is why the offset is what
centres a padded sprite on its collider. `SpriteSystem` reads `Rotation` from the same
entity and applies `scaleX`/`scaleY` when drawing.

Flipping is visual only: `flipX`/`flipY` mirror the drawing about the sprite's own centre.
`Component.flipX(entity)` (Lua) also mirrors the whole child subtree about the entity's
centre, so a rig turns around in one call.

## Shapes

`ShapeComponent` supports rectangles, circles, regular polygons, and lines:

```cpp
Engine::ShapeComponent body;
body.type = Engine::ShapeType::RECTANGLE;
body.width = 48.0f;
body.height = 32.0f;
body.color = SKYBLUE;
body.filled = true;
```

For polygons, set `sides` and `radius`. For lines, set `endOffset` and `thickness`.

## Text

`TextPlugin` registers `Text` and `TextSystem`. Attach text with:

```cpp
Engine::TextPlugin::AttachToEntity(registry, entity, "Score: 0", 24.0f);
```

Custom fonts, colours, and spacing are set directly on the `Text` component. If
`font.texture.id == 0` (no custom font loaded), the system falls back to Raylib's built-in
font. Entities without a `Position` are skipped safely.

## Camera

A camera is a normal entity with a `CameraComponent` and a `Position`. The active camera is
the one with `primary = true` and the lowest entity id. `CameraSystem` handles follow,
bounds, zoom, rotation, and shake.

```cpp
Engine::CameraComponent cam;
cam.followTarget = player;
cam.followSpeed = 5.0f;       // 0 snaps instantly
cam.clampToBounds = true;
cam.bounds = {0.0f, 0.0f, levelWidth, levelHeight};
cam.zoom = 2.0f;
cam.shakeTrauma = 1.0f;       // shake strength is magnitude * trauma^2
```

Shake is visual only and never moves the camera's `Position`. The clamp ignores rotation.

HUD elements use the `ScreenSpace` tag and are drawn in window pixels. Convert between
screen and world coordinates with `CameraSystem::ScreenToWorld` and `WorldToScreen`.

## Virtual resolution

`VirtualResolution` is a fixed-size game canvas scaled and centred into the OS window with
letterboxing or pillarboxing. The mouse can be mapped back into canvas space. Set
`virtualWidth`/`virtualHeight` in `ApplicationConfig`; `0` means match the window.

## Virtual canvas and editor

The editor's Scene view renders the registry with the same systems and camera, at the
project's virtual resolution, so what you see is what the game draws. The editor camera is
separate and can be panned and zoomed without affecting the game camera.

See [Camera](rendering.md) in the old guides and [Engine → Assets](assets.md).
