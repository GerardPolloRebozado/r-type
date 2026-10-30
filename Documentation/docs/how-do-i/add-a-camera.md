# Add a camera

1. Create an entity.
2. Add a `CameraComponent` with a follow target.

```cpp
Engine::Entity camera = registry.CreateEntity();
Engine::CameraComponent cam;
cam.followTarget = player;
cam.followSpeed = 5.0f;       // 0 snaps instantly
cam.clampToBounds = true;
cam.bounds = {0.0f, 0.0f, levelWidth, levelHeight};
cam.zoom = 1.0f;
Engine::CameraPlugin::AttachToEntity(registry, camera, cam);
```

The camera is a normal entity: move it by moving its `Position`, rotate it via `Rotation`.
Only one camera renders  -  the one with `primary = true` and the lowest entity id.

For shake, set `cam.shakeTrauma = 1.0f`; strength is `shakeMagnitude * trauma^2` and trauma
decays by `shakeDecay` per second. Shake is visual only.

Mouse input is in window pixels; convert with `CameraSystem::ScreenToWorld` before comparing
to world positions.

See [Engine → Rendering](../engine/rendering.md) and [Collisions](../engine/collisions.md)
for world vs. screen coordinates.
