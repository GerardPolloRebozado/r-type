# Headless mode

Headless mode runs the engine without a window or rendering. Systems and networking still
tick, which is how the multiplayer server runs.

## Enabling

Set `config.headless = true`:

```cpp
Engine::ApplicationConfig config;
config.headless = true;
Engine::Application app(config);
app.Run();
```

Or pass `--headless` to a generated game, which `MultiplayerPlugin` reads from the launch
options.

## What changes

- `InitWindow` is not called; no OpenGL context is created.
- `VirtualResolution` is not registered, so there is no canvas.
- `PostUpdate` returns immediately: no rendering, no `onUI`.
- `PreUpdate` skips virtual-canvas scaling and mouse mapping, but still polls input.
- `Update` runs every registered system in order, including `ScriptSystem` and
  `MultiplayerSystem`.

## What stays the same

- The fixed timestep loop and `dt`.
- Component, system, and resource registration.
- Scene loading through `SceneManager`.
- Networking: the server host is created and polls every tick.

## Use cases

- Dedicated multiplayer servers (`Game --server --headless`).
- Automated tests that exercise gameplay without a window.
- Batch processing or headless simulation.

## Caveats

- Systems that assume a window (rendering, ImGui) must guard with `HasResource<VirtualResolution>()`.
- `GetGameTexture()` throws `ErrorEcs` in headless mode because the resource is missing.
- Audio may still open a device; disable it by skipping `AudioPlugin::Init()` if you do not
  need sound on a server.
