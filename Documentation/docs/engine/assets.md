# Asset Management

The `AssetManager` is a global resource registered in the ECS. 

## Loading Assets
```cpp
auto& assets = registry.GetResource<Engine::AssetManager>();
assets.LoadSound("explosion", "assets/boom.wav");
assets.LoadMusic("bgm", "assets/music.mp3", true);
```
