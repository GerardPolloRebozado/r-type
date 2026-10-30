# Audio (component)

## `Component.addAudioSource`

```lua
Component.addAudioSource(entity, path, isMusic?, loop?, volume?) -> bool
```

Defaults: `isMusic = false`, `loop = false`, `volume = 1`.

## `Component.getAudioSource`

```lua
Component.getAudioSource(entity) -> {path, isMusic, loop, volume, isPlaying} | nil
```

## `Component.setAudioSource`

```lua
Component.setAudioSource(entity, path, isMusic?, loop?, volume?) -> bool
```

## `Component.playAudio`

```lua
Component.playAudio(entity) -> bool
```

## `Component.stopAudio`

```lua
Component.stopAudio(entity) -> bool
```

## `Component.setAudioVolume`

```lua
Component.setAudioVolume(entity, volume) -> bool
```

## `Component.setAudioLoop`

```lua
Component.setAudioLoop(entity, loop) -> bool
```
