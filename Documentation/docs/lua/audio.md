# Audio

Namespace `Audio` for global sound and music. Component-level audio functions are in
[Component → Audio](component/audio.md).

## `Audio.playSound`

```lua
Audio.playSound(path, volume?, loop?) -> bool
```

`volume` defaults to 1, `loop` to `false`.

## `Audio.playMusic`

```lua
Audio.playMusic(path, volume?, loop?) -> bool
```

## `Audio.stopSound`

```lua
Audio.stopSound(path) -> nil
```

Swallows errors if the asset is missing.

## `Audio.stopMusic`

```lua
Audio.stopMusic(path) -> nil
```

Swallows errors if the asset is missing.

## `Audio.stopAllSounds`

```lua
Audio.stopAllSounds() -> nil
```

## `Audio.stopAllMusic`

```lua
Audio.stopAllMusic() -> nil
```

## `Audio.setSoundVolume`

```lua
Audio.setSoundVolume(path, volume) -> bool
```

## `Audio.setMusicVolume`

```lua
Audio.setMusicVolume(path, volume) -> bool
```

## `Audio.isSoundPlaying`

```lua
Audio.isSoundPlaying(path) -> bool
```

## `Audio.isMusicPlaying`

```lua
Audio.isMusicPlaying(path) -> bool
```

Volumes are clamped to 0..1.
