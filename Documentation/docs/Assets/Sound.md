# Audio System Documentation

Audio assets are managed globally, and entities trigger sounds simply by changing data components. The engine
automatically handles multiple overlapping sounds (cloning/aliasing) and background music streaming.

## Overview

1. **AssetManager (Resource):** Stores the actual heavy audio files in memory.
2. **AudioSource (Component):** A lightweight data struct attached to your entities.
3. **AudioSystem (System):** Reacts to your components and talks to the audio hardware.
4. **AudioPlugin (Plugin):** A helper class to automatically register all of the above.

---

## 1. Setup

Before playing audio, you just need to initialize the `AudioPlugin`! This will automatically register the `AssetManager`
resource, the `AudioSource` component, and the `AudioSystem` into your ECS.

```cpp
#include "Core/Application.hpp"
#include "Plugins/AudioPlugin.hpp"

// Inside your main function or engine initialization:
Engine::AudioPlugin audioPlugin;
audioPlugin.Init(app.GetRegistry());
```

---

## 2. Loading Audio

Audio is split into two types:

* **Music:** Long background tracks (e.g., MP3s). Streamed from disk to save RAM.
* **Sound:** Short effects (e.g., lasers, footsteps, explosions). Loaded into RAM so they can be played instantly and
  overlapped many times.

### Recommended: Loading from Memory (Bundled Assets)

By far the easiest way to manage audio is to drop your files into your game's `assets/` folder. The `AssetBundler` will
automatically convert them into a C++ header when you compile. **This is highly recommended as it compiles all assets
directly into your executable, making distributing your binaries much easier (no missing files!).**

1. Ensure your `CMakeLists.txt` is configured to run the `AssetBundler` on your assets.
2. Include the generated header in your code and register everything at once:

```cpp
#include "BundledAssets.hpp"

// Inside your main function:
auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();

// Automatically loads EVERYTHING in your assets/ folder directly from memory!
BundledAssets::RegisterAssets(assets);

// The alias (assetId) is simply the relative filename (e.g., "laser.wav" or "music.mp3").
// Since memory-loaded music doesn't loop by default, set looping manually if needed:
if (assets.GetAllMusic().contains("music.mp3")) {
    assets.GetMusic("music.mp3").SetLooping(true);
}
```

### Alternative: Loading from Disk

If you have massive audio files and prefer not to bundle them into the executable, you can still load them from the disk
manually:

```cpp
auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();

// Load a background music track
assets.LoadMusic("bgm_level1", "assets/music.mp3", true); // true = loop

// Load a sound effect
assets.LoadSound("laser_pew", "assets/laser.wav");
```

---

## 3. Playing Sounds

Because this is a pure ECS, you **do not** call `.Play()` directly. Instead, you attach an `AudioSource` component to an
entity and set `shouldPlay = true`. The `AudioSystem` will detect this and play the sound for you.

### Example: Spawning an entity that plays a sound immediately

```cpp
Engine::Entity laserEntity = app.GetRegistry().CreateEntity();

Engine::AudioSource audio;
audio.assetId = "laser_pew";
audio.isMusic = false; // It's a sound effect
audio.shouldPlay = true; // Trigger playback!

app.GetRegistry().AddComponent(laserEntity, audio);
```

### Example: Triggering a sound from a Gameplay System

If you have an entity that already has an `AudioSource` component (like a player ship), you can trigger the sound at any
time from your gameplay logic by modifying the data:

```cpp
class PlayerSystem : public Engine::System {
public:
    void Update(Engine::Registry& registry, float dt) override {
        for (auto entity : m_Entities) {
            auto& audio = registry.GetComponent<Engine::AudioSource>(entity);
            
            if (PlayerPressedShootButton()) {
                // Pure Data-Oriented action: Just change the state!
                audio.shouldPlay = true; 
            }
        }
    }
};
```

---

## 4. Background Music

Music is handled the exact same way. If an entity requests music to play, the `AudioSystem` will automatically
continuously update the streaming buffer in the background every frame.

```cpp
Engine::Entity levelEntity = app.GetRegistry().CreateEntity();

Engine::AudioSource bgm;
bgm.assetId = "bgm_level1";
bgm.isMusic = true; // Tell the system it is a music stream
bgm.shouldPlay = true;

app.GetRegistry().AddComponent(levelEntity, bgm);
```

To stop the music:

```cpp
auto& bgm = registry.GetComponent<Engine::AudioSource>(levelEntity);
bgm.shouldStop = true;
```

---

## Advanced: Overlapping Sounds (Cloning)

You don't need to worry about memory if multiple enemies shoot at the same time! The `AudioSystem` automatically creates
lightweight "Aliases" (clones) of your sound effects.

If 50 entities all have an `AudioSource` pointing to `"laser_pew"` and they all set `shouldPlay = true` on the same
frame, the engine will play 50 overlapping lasers perfectly, while only keeping one copy of the audio data in RAM.
