# Step-by-Step Tutorial

Welcome to the tutorial! Here you will learn how to initialize the engine, spawn an entity, and render it to the screen.

## 1. Initializing the Engine

```cpp
#include "Core/Application.hpp"

int main() {
    Engine::ApplicationConfig config;
    config.windowTitle = "My Game";
    config.targetFPS = 60;
    
    Engine::Application app(config);
    app.Run();
    return 0;
}
```
