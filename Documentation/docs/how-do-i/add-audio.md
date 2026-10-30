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

## Tips

- Sound effects are decoded into RAM and can overlap many times.
- Music is streamed from disk with one playhead per asset.
- Set `loop = true` for background tracks.
- `shouldPlay` is cleared by `AudioSystem` after handling; set it once, not every frame.
- Bundled ids are relative file names (`laser.wav`, not `assets/laser.wav`).

See [Engine → Audio](../engine/audio.md) and [Lua → Audio](../lua/audio.md).
