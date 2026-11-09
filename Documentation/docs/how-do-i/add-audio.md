# Add audio

## Play a sound from Lua

```lua
Component.addAudioSource(entity, "assets/laser.wav", false, false, 1.0)
Component.playAudio(entity)
```

Or use the global `Audio` namespace:

```lua
Audio.playSound("assets/laser.wav", 1.0, false)
```

## Play music

```lua
Audio.playMusic("assets/music.mp3", 0.6, true)
```

Stop it with `Audio.stopMusic("assets/music.mp3")` or `Audio.stopAllMusic()`.

## Settings screen

The settings template (`UI/lua/templates/settings.lua`) always draws `VOL -`, `VOL +`, `Mute music`, and `Mute effects` above any extra rows.

- `VOL -` and `VOL +` add or subtract `0.1` on each `AudioSource`, clamped to `0..1`. Sources keep their own levels.
- `Mute music` sets every source with `isMusic` to `0`. Unchecking it writes the volume saved when that source was muted.
- `Mute effects` does the same for sources with `isMusic == false`.
- A category with no sources stays as it is. While a mute stays checked, each draw of the settings screen keeps that category at `0`, including a source that appears while the screen is open.
- The editor preview draws these rows and does not write volume into the scene.
- `Audio.playSound` and `Audio.playMusic` are not `AudioSource` components, so these rows do not change them.

## Tips

- Sound effects are decoded into RAM and can overlap many times.
- Music is streamed from disk with one playhead per asset.
- Set `loop = true` for background tracks.
- `shouldPlay` is cleared by `AudioSystem` after handling; set it once, not every frame.
- Bundled ids are relative file names (`laser.wav`, not `assets/laser.wav`).

See [Engine → Audio](../engine/audio.md) and [Lua → Audio](../lua/audio.md).
