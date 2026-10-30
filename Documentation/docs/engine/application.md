# Application

`Engine::Application` owns the window, the `Registry`, and the main loop. You create one in
`main()`, register plugins, spawn entities, and call `Run()`.

## Configuration

`Engine::ApplicationConfig` is an aggregate with sensible defaults:

| Field | Default | Notes |
|---|---|---|
| `windowWidth` / `windowHeight` | `1280` / `720` | OS window size |
| `virtualWidth` / `virtualHeight` | `0` | `0` means "match the window", so the game runs 1:1 until you set a canvas size |
| `windowTitle` | `"Game Engine"` | Window caption |
| `maximized` | `false` | Adds the maximized window flag; the window is always resizable |
| `targetFPS` | `60` | Fixed simulation step rate |
| `headless` | `false` | No window, no rendering; systems and networking still tick |
| `args` | empty | Command-line arguments; published as `LaunchOptions` and to Lua as `arg` |
| `onUI` | none | Called after the canvas is presented; used by the editor for panels |
| `customRender` | none | When set, the editor draws the registry into its own surface |

## The loop

`Run()` is a fixed-timestep loop:

```text
while running:
    PreUpdate(dt)          input polling, virtual-canvas scaling, mouse mapping
    while lag >= step:
        Update(dt)         every registered system, in registration order
    PostUpdate(dt)         rendering: world pass, then screen pass
    FlushQueues()          deferred entity destruction
    ApplyPendingScene()     switch scenes requested during the frame
```

The step is `1.0 / targetFPS` seconds. Gameplay multiplies by `dt`, so it is framerate-independent.

## Accessing the registry

```cpp
Engine::Application app(config);
Engine::Registry& registry = app.GetRegistry();
```

The registry is created inside `Application`; components, systems, and resources are added
through it. `app.Quit()` stops the loop from anywhere, including from a system.

## Headless mode

Set `config.headless = true` (or pass `--headless` to a generated game) to run without a
window. `InitWindow` is skipped, `VirtualResolution` is not registered, and `PostUpdate`
does nothing, but `Update` and `PreUpdate` still run. Networking and the `SceneManager`
keep working, which is how the multiplayer server runs.

See [Headless mode](headless.md).

## Window and canvas

The OS window and the virtual canvas are separate. `VirtualResolution` scales a fixed-size
render texture into the window with letterboxing or pillarboxing, and maps the mouse into
canvas space. Set `virtualWidth`/`virtualHeight` to a fixed resolution (for example
`1920x1080`) to get a stable coordinate system regardless of window size.

See [Rendering](rendering.md).

## Scene switching

`SceneManager` is a resource that queues a scene change. `ApplyPendingScene()` runs after
`FlushQueues`, so destruction from the previous scene is complete before the new one loads.
Lua scripts use `Scene.load(name)`; clients in multiplayer ignore their own request and
follow the server.
