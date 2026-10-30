# Audio

Audio is fully ECS-driven: gameplay sets fields on an `AudioSource` component and
`AudioSystem` talks to the device. You never call `Play()` from gameplay code.

## Setup

```cpp
Engine::AudioPlugin audioPlugin;
audioPlugin.Init(app.GetRegistry());
```

That registers `AssetManager` (if missing), `AudioSource`, and `AudioSystem`, and opens
the audio device.

## Loading audio

There are two asset kinds:

| Kind | Use for | Behaviour |
|---|---|---|
| **Sound** | Lasers, hits, explosions | Decoded into RAM, can overlap many times |
| **Music** | Background tracks | Streamed from disk, one playhead per asset |

From bundled assets (recommended):

```cpp
#include "BundledAssets.hpp"

auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();
BundledAssets::RegisterAssets(assets);
if (assets.GetAllMusic().contains("music.mp3")) {
    assets.GetMusic("music.mp3").SetLooping(true);
}
```

From disk:

```cpp
auto& assets = app.GetRegistry().GetResource<Engine::AssetManager>();
assets.LoadMusic("bgm_level1", "assets/music.mp3", true); // true = loop
assets.LoadSound("laser_pew", "assets/laser.wav");
```

Bundled ids are relative file names (`laser.wav`, not `assets/laser.wav`).

## Playing a sound

Attach the component and set `shouldPlay`. `AudioSystem` clears the flag when it has handled
the request.

```cpp
Engine::AudioSource audio;
audio.assetId = "laser_pew";
audio.isMusic = false;
audio.shouldPlay = true;
registry.AddComponent(laser, audio);
```

Or trigger from a gameplay system by flipping the field. This is the data-oriented pattern:
gameplay mutates data, the audio system observes it.

## Background music

```cpp
Engine::AudioSource bgm;
bgm.assetId = "bgm_level1";
bgm.isMusic = true;
bgm.shouldPlay = true;
registry.AddComponent(levelEntity, bgm);
```

Stop it with `shouldStop = true`. `AudioSystem` updates the streaming buffer every frame
while a track plays.

## Overlapping sounds

You do not manage voices. If 50 entities request `laser_pew` on the same frame, the asset
manager hands out 50 lightweight aliases and `AudioSystem` mixes them, while keeping one
decoded copy of the file in RAM. The alias list is pruned as sounds finish.

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| Nothing plays | `AudioPlugin::Init()` was not called, or the device failed to open |
| `GetResource` throws | `AudioPlugin` was skipped and `AssetManager` was never registered |
| Wrong asset id | Bundled ids are relative paths |
| Music restarts every frame | `shouldPlay` is being set continuously instead of once |
| Sound cut off | The entity or its `AudioSource` was destroyed |
