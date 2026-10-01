# State Management

States are global variables that trigger events when they change

## The `State` and `NextState` Resources

The state system separates the *current* state from the *requested* state.

* **`State<T>`**: A read-only resource representing the current state of the game. Systems read this to know what state
  the game is in.
* **`NextState<T>`**: A writable resource used to queue a transition for the next frame.

### Setting Up States

Initialize the states by adding the resources to the registry at startup:

```cpp
enum class GameState { MainMenu, Playing, Paused };

// Set initial state to MainMenu
registry.AddResource(Engine::State<GameState>(GameState::MainMenu));

// Initialize the empty queue
registry.AddResource(Engine::NextState<GameState>());
```

### Transitioning States

To change states we simply request the `NextState` and we call it set function

```cpp
auto& nextState = registry.GetResource<Engine::NextState<GameState>>();
nextState.set(GameState::Playing);
```

The internal `StatesSystem` will process this request at the end of the frame, update `State<GameState>`, and clear the
queue.

---

## Triggering Logic on State Change (OnEnter / OnExit)

When the `StatesSystem` processes a transition, it emits a one-frame `StateTransitionEvent<T>` resource. You can use
this to trigger specific logic exactly when a state changes, such as despawning UI or spawning the player.

### Example: Despawning the Main Menu

A common requirement is despawning all menu entities when leaving the `MainMenu` state.

**1. Create a Tag Component:**
First, we create an empty struct to tag entities that belong to the menu.

```cpp
struct MainMenuEntity {};
```

Tag your entities when you spawn them:

```cpp
Entity button = registry.CreateEntity();
registry.AddComponent(button, MainMenuEntity{});
```

**2. Write a Cleanup System:**
Create a system that runs every frame but only executes its logic if the transition event indicates we just exited the
`MainMenu`.

```cpp
class MainMenuCleanupSystem : public Engine::System {
public:
    void Update(Engine::Registry &registry, float dt) override {
        // Ensure the event resource exists
        if (!registry.HasResource<Engine::StateTransitionEvent<GameState>>()) return;
        
        auto &event = registry.GetResource<Engine::StateTransitionEvent<GameState>>();

        // Check if we JUST exited the MainMenu state this frame
        if (event.exited.has_value() && event.exited.value() == GameState::MainMenu) {
            
            // Find all entities tagged as MainMenuEntity and destroy them
            auto view = registry.GetComponentsArray<MainMenuEntity>();
            for (auto entity : view) {
                registry.QueueDestroyEntity(entity);
            }
        }
    }
};
```

This data-driven approach keeps your logic fully decoupled and avoids storing raw function callbacks inside your ECS
resources.
