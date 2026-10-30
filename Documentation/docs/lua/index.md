# Lua

Gameplay in Lucky & Pozzo is written in Lua. These pages document every namespace and
function exposed to scripts, with exact signatures, return shapes, defaults, and gotchas.

- [Entity](entity.md)  -  create, destroy, find, name.
- [Transform](transform.md)  -  position, rotation, draw layer.
- [Component](component.md)  -  sprites, shapes, text, colliders, opacity, hierarchy, connections, audio, scripts.
- [Input & Controller](input.md)  -  keyboard, mouse, keybinds, actions.
- [Time](time.md)  -  timing.
- [Scene](scene.md)  -  scene switching.
- [Log](log.md)  -  logging.
- [Layers](layers.md)  -  collision layer names and masks.
- [Audio](audio.md)  -  global sound and music.
- [Net](net.md)  -  multiplayer setup, messaging, replication, RPCs.

Conventions common to all pages:

- `Entity` handles are plain numbers in Lua.
- Most `Component.*` functions return `bool` success (`false` on missing component or dead entity).
- Functions taking `sol::optional` arguments omit them as `nil`.
- `Transform.getPosition` returns a fresh table `{x, y, z}`.
- `Component.getSprite` returns the literal string `"sprite"` if a sprite exists, else `nil`;
  use `getSpriteInfo` for data.
- `log`/`print`: `print(...)` converts each vararg to a string, joins with tabs, and logs
  via `log_info`. Non-string args throw.
- `Time.deltaTime()` is a fallback constant `1/60`; prefer the `dt` passed to `onUpdate`.
- `Scene.load` is ignored on clients in multiplayer (the server drives scene changes) and
  errors if no `SceneManager`.
- `Component.has` / `remove` / `entitiesWith` accept friendly alias names only:
  `Position`, `Rotation`, `Sprite`, `Shape`, `Text`, `Name`, `Collider`, `Opacity`,
  `Parent`, `AudioSource`.

See also: [Engine → Lua scripting](../engine/scripting.md), [Plugins → Lua plugins](../plugins/lua.md).
