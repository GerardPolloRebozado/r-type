# Multiplayer

Preview and test networked scenes.

<<<<<<< HEAD
!!! note "Stub"
    This page is a placeholder. The content is being rewritten for the new
    documentation structure.
=======
> **In plain words:** the editor is the drawing board. When you hit **Play Multiplayer**, it starts a
> referee (the server) and a few players (the clients) as separate windows, all looking at the
> same drawing. You close them all with **Stop**.

For what happens *inside* those processes, read the [Engine → Networking](../engine/networking.md)
page. This page is only about using the feature.

---

## 1. Where it lives

Two places in the editor:

| Control | Location | What it does |
|---|---|---|
| **Play Multiplayer** (people icon) / **Stop** | main toolbar, right of the single-player Play button | launches / kills the whole playtest |
| **Client Count** | Settings panel → *Multiplayer Playtest* | how many client windows to launch (1–5) |
| **Headless Server** | Settings panel → *Multiplayer Playtest* | run the server with no window |

The toolbar buttons, left to right:

```
[ 🔨 Build ]   [ ▶ Play ]   [ 👥 Play Multiplayer ]   (or)   [ ⏹ Stop ]
```

- **▶ Play** launches a single, single-player process.
- **👥 Play Multiplayer** launches `1 server + N clients`.
- **⏹ Stop** (replaces the play buttons while games run) kills every launched process.

---

## 2. The settings

Settings panel → **Multiplayer Playtest**:

| Setting | Range | Meaning |
|---|---|---|
| **Client Count** | `1`–`5` | how many client processes to launch; clamped to `PlayerSlotState::kMaxPlayers` |
| **Headless Server** | on/off | adds `--headless` to the server, so it runs without a window |

The count is capped at **5** because the engine assigns one distinct **color slot** per connected
player (`0`–`4`). The transport also enforces it, so a sixth client is simply refused. Player `N`
gets color slot `N-1`; in the shipped R-Type project that maps to sprite block `5N-4 .. 5N`.

The server address and port are **not** exposed here yet — a launched client always connects to
`127.0.0.1:3000` (see [§8](#8-missing-steps-not-available-yet)).

---

## 3. The **Network** component in the Inspector

Select an entity (or select a `.prefab` in the Assets panel) and the Inspector shows a **Network**
section when the entity has a `NetworkComponent`. Add it from the component menu like any other.

| Field | Control | Meaning |
|---|---|---|
| **Authority** | dropdown: `Server` / `Client` / `Shared` | who simulates the entity (see below) |
| **Sync Transform** | checkbox | replicate `Position` + `Rotation` every tick |
| **Sync Sprite** | checkbox | replicate the sprite (path, offset, scale, flips) |
| **Prefab Path** | text field + drag-and-drop target | prefab to instantiate on the other machine when spawning |
| **Network ID** | read-only label | engine-assigned; shows *(assigned on connect)* until then |

You can type a prefab path or **drag a `.prefab` from the Assets panel** straight onto the field.

### Choosing an authority

| Authority | Means | Use it for |
|---|---|---|
| **Server** | the server simulates it; clients follow | enemies, pickups, moving platforms, hazards |
| **Client** | one client sends input; the server simulates it | each connected player |
| **Shared** | not synced at all; built identically from the scene | walls, floors, background |

> A `Client` entity in the scene is a **template**: the server destroys it on startup and spawns one
> copy per connected player from **Prefab Path**. So you author the player **once**, set the prefab
> path, and every connection gets its own instance — with its own color slot.

---

## 4. What the button actually launches

When you click **Play Multiplayer** with a scene open and a client count of 3, the editor spawns:

```bash
Game --server <scene> [--headless]     # the server
Game --client <scene>                  # client 1
Game --client <scene>                  # client 2
Game --client <scene>                  # client 3
```

Each process is started in its own **process group** so **Stop** can kill the whole tree without
touching the editor. On Windows the same happens through `CreateProcessA` / `TerminateProcess`.

The processes themselves parse these flags in `MultiplayerPlugin::Init` (see the
[engine page](../engine/networking.md#2-choosing-a-role-at-startup)); `--headless` additionally
disables the window and rendering.

---

## 5. Quick start: a multiplayer scene in five minutes

This is the minimal recipe. It assumes one player prefab and one enemy.

**1. Author the player as a prefab.**

- Build the entity in the scene: `Position`, `SpriteComponent`, a collider, and whatever script
  drives it (`player.lua`).
- Drag it into the Assets panel to write `Player.prefab`.

**2. Make it networked.**

- Select `Player.prefab` (or the entity) and add a **Network** component.
- Set **Authority → Client**, tick **Sync Transform** and **Sync Sprite**.
- Set **Prefab Path → `Prefabs/Player.prefab`** (drag the prefab in).

**3. Place the template in the scene.**

- Keep one instance of the player in the scene. It is only a template; the server will replace it
  with one live player per connection. Don't script it to depend on being "The Player".

**4. Author the enemy.**

- Add a **Network** component to the enemy (or its prefab) with **Authority → Server**,
  **Sync Transform** on. Because it exists in the scene on every machine, it needs no prefab path.

**5. Build, then play.**

- Click **🔨 Build** (Play Multiplayer does **not** build for you — see [§8](#8-missing-steps-not-available-yet)).
- Set **Client Count** to `2` in Settings.
- Click **👥 Play Multiplayer**. Two client windows appear and connect; the tail of the server log
  shows the clients connecting.
- Click **⏹ Stop** when done.

Your game code stays ordinary: `player.lua` reads `Net.input(entity)`, moves, shoots; the engine
handles the rest. In single-player the same script falls back to the local controller
(`Net.isNetworked()` is false).

---

## 6. How flexible is it? Examples

The mechanism is genre-agnostic: you are only choosing, per entity, **who owns it** and **what gets
synchronised**. Below are sketches for very different kinds of games, all using the same editor
controls.

### a. Side-scrolling shooter (the shipped R-Type example)

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| Player | `Client` | transform + sprite | one per connection, distinct color slot |
| Enemy | `Server` | transform (+ sprite if it animates) | exists in the scene on both sides |
| Bullet / explosion | `Server` | transform + sprite | created at runtime, marked with `Net.replicate` |
| Walls / background | `Shared` | — | identical everywhere, zero bandwidth |

```lua
-- playerGun.lua: fire and let the engine replicate the bullet
local bullet = Entity.create()
Transform.setPosition(bullet, x, y)
Net.replicate(bullet, "Server", true, true)
```

### b. Top-down arena brawler / party game

Same shape as (a) but with free 2D movement and pickups:

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| Player | `Client` | transform + sprite | each client controls its own |
| Pickup | `Server` | transform + sprite | server decides who grabs it |
| Arena tiles | `Shared` | — | level geometry |

Because the server owns the pickup, two players reaching it on the same frame resolves
deterministically — no client gets to "win" by touching first locally.

### c. Co-op action / physics playground

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| Player | `Client` | transform | |
| Crate / ball / lever | `Server` | transform | shared physics the server resolves |
| Door | `Server` | sprite | opened by an RPC when the lever is pulled |

Use `Net.rpc` for discrete interactions ("open the door"), and `Server` authority for anything both
players must agree on.

```lua
-- on the lever, server side
Net.rpc(leverEntity, "doorOpened", { id = self.doorId })
```

### d. Turn-based board / card game

Turn-based games barely need state streaming at all — they need **events**:

- Add a `Network` component with **Authority → Client** to each player's hand/avatar, or skip
  replication entirely and use RPCs.
- Turn actions are `Net.rpc(entity, ...)` calls (`playCard`, `endTurn`), and the server validates
  them and broadcasts the resulting state.
- Because a snapshot is sent every tick regardless, you can also just mark the board `Server` and
  let the state replicate; no interpolation issues when nothing moves.

### e. Racing / sports with several local players

| Entity | Authority | Sync |
|---|---|---|
| Car / athlete | `Client` | transform |
| Track decorations | `Shared` | — |
| Checkpoints / finish line | `Server` | — (triggers, resolved server-side) |

The server decides finishing order and lap counts; clients only ever report steering and throttle.

### f. Single-player still works

A scene with `Network` components still runs if launched **without** any network flag (the ordinary
**▶ Play** button, or `Game` with no arguments). No host is created and `Net.isNetworked()` is
false, so scripts take their single-player branch. You do not maintain two versions of a game.

### Summary table

| Genre | Who is `Client`-authority | Who is `Server`-authority | Discrete events |
|---|---|---|---|
| Shooter | players | enemies, bullets | shoot |
| Arena brawler | players | pickups, hazards | grab |
| Co-op action | players | physics objects | interact |
| Turn-based | (optional) hands | board, deck | play/end turn |
| Racing | vehicles | checkpoints | lap |
| Solitaire-like | — | — | all RPC |

---

## 7. Recipes for different genres

Deeper than the sketches above: for each genre, what to tag in the Inspector and the Lua pattern that
makes it work. Every recipe is **editor + Lua only** — no C++.

> Two facts shape all of them. **Only `Position` (+ `Rotation.angle`) and the sprite replicate
> declaratively**; anything else (health, money, turn, cards) travels as an RPC, or is encoded in a
> sprite path. And **a Lua script cannot instantiate a `.prefab`** — it builds bare entities with
> `Entity.create()` + `Component.add*`. The workaround: build on the server, then hand a `prefabPath`
> to `Net.replicate(...)` so *clients* still instantiate the authored prefab (see the
> [engine TODO](../engine/networking.md#14-todos-and-planned-work)).

Two blocks reused everywhere:

```lua
-- true in single-player and on the server; false on a client
local function shouldSimulate()
    if not Net.isNetworked() then return true end
    return Net.isServer()
end
```

```lua
-- Server: create a runtime object; clients instantiate the authored prefab
local e = Entity.create()
Transform.setPosition(e, x, y)
Component.addSprite(e, "assets/thing.png", 0, 0, 2, 2)
Component.addBoxCollider(e, 24, 24)
Net.replicate(e, "Server", true, true, "Prefabs/Thing.prefab")
```

### 7.1 Pong / air-hockey — real-time, almost no state

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| `Ball` (scene) | Server | Transform | the server moves it in Lua |
| `Paddle` prefab | Client | Transform | one per connection (slots 0, 1) |
| Goal / wall | Shared | — | identical everywhere |

Paddle (server side):

```lua
function Paddle:onUpdate(e, dt)
    if not shouldSimulate(e) then return end
    local i = Net.input(e) or { UP = Controller.isActionActive("UP"),
                                DOWN = Controller.isActionActive("DOWN") }
    local dy = (i.DOWN and 1 or 0) - (i.UP and 1 or 0)
    Transform.translate(e, 0, dy * SPEED * dt)
end
```

The ball needs **no networking code at all** — it is `Server` authority, so `syncTransform` carries
it to both clients. Score is the only extra state: `Net.broadcast("score", { l = .., r = .. })` on the
server, `Net.on("score", ...)` on clients.

**Why it fits:** the simplest proof that a moving object "just appears" on every screen.

### 7.2 Top-down arena shooter — bullets, health, respawn

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| `Player` prefab | Client | Transform + Sprite | color from slot |
| `Bullet` (runtime) | Server | Transform + Sprite | server builds it; clients get the prefab |
| Wall | Shared | — | |

Firing uses the `Net.replicate(..., prefabPath)` pattern. Health is **not** synced, so the server
announces it:

```lua
-- server, on hit
Net.rpc(victim, "hurt", { hp = newHp })
-- client handler
Net.onEntityRpc("hurt", function(e, data) self.hp = data.hp; self.flash = 0.2 end)
```

`player_input` carries the whole set of active action names marked `networked`, so extra actions
(dash / reload) work on the server too — no extra RPC needed as long as the keybind is networked:

```lua
if Net.isServer() and input.DASH then Net.rpc(e, "dash", {}) end
```

Respawn should **reuse** the player entity rather than destroy and re-create it: the server binds a
player's identity and color slot when it spawns them, and a plain `Entity.destroy` from a script does
**not** release that binding or the slot (only a disconnect does, or an explicit `Net.untrack`). So
reset in place:

```lua
Transform.setPosition(e, spawnX, spawnY)
Component.setColliderEnabled(e, false)
Component.setOpacity(e, 0)
```

**Why it fits:** a twitch shooter with health, respawn and score, from just `Net.replicate` +
`Net.rpc`.

### 7.3 Co-op platformer — a server-driven world

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| `Player` prefab | Client | Transform + Sprite | |
| `Platform` prefab | Server | Transform | the server animates it in Lua |
| `Lever` / `Door` | Server | Sprite | sprite swaps on/off; the door slides via a broadcast |
| Terrain | Shared | — | |

Continuous state — the server animates, clients just watch:

```lua
function Platform:onUpdate(e, dt)
    if not shouldSimulate(e) then return end
    self.t = (self.t or 0) + dt
    Transform.setPosition(e, self.x0, self.y0 + math.sin(self.t) * 60)
end
```

Discrete event — a lever is an RPC, not a stream:

```lua
-- lever, server
Component.setSpritePath(e, self.on and ON_IMG or OFF_IMG)
Net.broadcast("door", { open = self.on })
```

**Why it fits:** the two flavours side by side — replicate a transform (continuous) vs. announce an
event (discrete).

### 7.4 Turn-based card / board game — RPCs and sprites

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| `Seat` prefab (invisible) | Client | none | gives each client an identity + slot |
| `CardSlot` × N | Server | Sprite | the sprite path **is** the card |
| HUD | none | — | local text per client |

Encode categorical state in the sprite path and let Sync Sprite do the work:

```lua
-- server, when a card is revealed
Component.setSpritePath(cardSlot, "cards/ace_spades.png")  -- clients update automatically
```

Moves are RPCs, validated on the server (which owns the rules in a script's `self`):

```lua
Net.rpc(seat, "playCard", { slot = 3 })                   -- client intent
Net.onEntityRpc("playCard", function(seat, data)           -- server authority
    if self.turnSlot ~= Net.playerSlot(seat) then return end
    -- ... validate the card ...
    self.turnSlot = (self.turnSlot + 1) % Net.maxPlayers()
    Net.broadcast("turn", { slot = self.turnSlot })
end)
```

**Why it fits:** nothing moves, so there is no tick, no interpolation and no cheating surface; the
server is simply the only rule-keeper.

### 7.5 Co-op horde / tower-defense — many server units + an economy

| Entity | Authority | Sync | Notes |
|---|---|---|---|
| `Player` prefab | Client | Transform + Sprite | |
| `Director` (empty + script) | none | — | the server spawns waves |
| `Zombie` (runtime) | Server | Transform + Sprite | `Net.replicate(..., "Prefabs/Zombie.prefab")` |
| `Tower` (runtime) | Server | Transform | placed via RPC |
| Path / terrain | Shared | — | |

The server director spawns a wave:

```lua
for i = 1, waveSize do
    local z = Entity.create()
    Transform.setPosition(z, spawnX, spawnY + i * 40)
    Component.addSprite(z, "assets/zombie.png", 0, 0, 3, 3)
    Component.addBoxCollider(z, 40, 40)
    Component.addScript(z, "scripts/zombie.lua")
    Net.replicate(z, "Server", true, true, "Prefabs/Zombie.prefab")
end
```

Building goes client → server → everyone:

```lua
Net.rpc(seat, "build", { x = mx, y = my, kind = "gun" })   -- client
Net.onEntityRpc("build", function(seat, d)                  -- server validates + spends
    if self.money < COST then return end
    self.money = self.money - COST
    local t = Entity.create()
    -- ... place + dress the tower ...
    Net.replicate(t, "Server", true, false, "Prefabs/Tower.prefab")
    Net.broadcast("money", { v = self.money })
end)
```

**Why it fits:** 5 humans, dozens of replicated AI and a shared economy, from two verbs
(`Net.replicate`, `Net.broadcast`).

### 7.6 The palette, at a glance

| You need… | Editor / Lua |
|---|---|
| A thing everyone sees move | `NetworkComponent` **Server** + Sync Transform |
| One entity per player | `NetworkComponent` **Client** + Prefab Path (scene template) |
| Something that never changes | **Shared** |
| A bullet / effect made at runtime | `Entity.create` + `Net.replicate(..., "Prefabs/…")` |
| Health / score / money / turn | `Net.broadcast` + `Net.on` |
| An action beyond WASD + fire | `Net.rpc(localPlayer, "action", data)` → `Net.onEntityRpc` |
| Categorical state without a packet | encode it in the sprite path + Sync Sprite |
| A per-player identity / color | `Net.playerSlot` / `Net.localEntity` |

### 7.7 Several scenes — lobby → level → results

The server decides which scene everyone plays. Call `Scene.load` on the server only; every client
follows automatically.

```lua
-- lobby.lua (on any entity of Lobby.scene)
function Script:onUpdate(entity, dt)
    if Net.isServer() and everyoneReady() then
        Scene.load("levels/Level1.scene")
    end
end
```

What happens:

1. At the end of that frame the **server** switches scenes and sends a reliable `scene_load`
   message to every client.
2. Each **client** loads the same scene. Packets that arrive after `scene_load` wait until the
   client has switched, so nothing from the new scene lands in the old one.
3. The server **respawns every connected player** in the new scene with the **same network id and
   color slot** (`Net.playerSlot`, `Net.localEntity` keep working).
4. A client that joins later, or that was started on another scene, is moved to the server's
   current scene when it connects.

Rules of thumb:

- `Scene.load` on a client is ignored (scripts often run on every machine).
- Every machine needs the scene files: they ship in the same project folder.
- Data that must survive a switch (scores, who is ready) lives in Lua **globals** on the server,
  then is sent again with `Net.broadcast` once the new scene starts. Handlers registered with
  `Net.on` stay registered; register them again in the new scene's `onCreate` if they use `self`.
- Reloading the current scene (`Scene.load(Scene.current())`) works too: everyone reloads it.

---

## 8. Missing steps (not available yet)

The multiplayer path is complete **end to end** for local playtests, but several conveniences are
still missing. This list is the honest "what you cannot do yet".

### Playtest workflow

- **Play Multiplayer does not build.** If the game executable is missing or stale, the button does
  nothing (the processes fail to launch silently). Click **🔨 Build** first.
- **Port and host are not in the UI.** Clients always connect to `127.0.0.1:3000`; changing that
  means editing `main.cpp` or launching from a terminal with `--host` / `--port`.
- **No per-client configuration.** Every client gets identical arguments.

### Netcode & gameplay feel

- **No client-side prediction.** Input has at least one tick of latency, even on localhost. This is
  the biggest "feel" gap for fast games.
- **No interpolation.** Remote entities move at the snapshot rate and can look steppy.
- **No lobby, matchmaking, server browser, password or authentication.** You connect to a host and
  port; the server trusts the input it receives.
- **No reconnect / session resume.** A dropped client is simply gone (its color slot is freed).
- **Hard cap of 5 players**, baked into both the slot resource and the transport (a
  [TODO](../engine/networking.md#14-todos-and-planned-work) to make it configurable).

### Tooling

- **No in-editor play mode** and **no script execution in the editor** (by design — scripts only run
  in the launched game).
- **No network inspector.** There is no panel for peers, bandwidth, packet log or per-entity sync
  rates.
- **No simulated latency / jitter / packet loss** for testing poor connections.
- **No hot reload.** Code changes require a rebuild before the playtest reflects them.

### Design constraints (things to design around)

These are not bugs — they are the current shape of the system, and a game has to fit them.

- **One player prefab per scene.** The engine keeps a *single* player prefab path (the first
  `Client`-authority template's **Prefab Path** in the current scene) and destroys every `Client`
  template when the scene loads, so every connection spawns that same prefab. A scene without a
  template keeps the previous scene's prefab. Two different player classes in one scene are not
  expressible today — give everyone the same prefab and vary look/behaviour per color slot
  (`Net.playerSlot`).
- **No private messages from Lua.** `Net.broadcast` reaches everyone, and Lua cannot turn a player
  entity into its peer to `Net.send` to a single client. Anything a player must not see (a hidden
  hand, fog of war) cannot be withheld through the API — only by not sending it.
- **The input packet is a fixed 5-bit set** (`up/down/left/right/shoot`). Any other button must go
  through an RPC (see §7.2); a sixth input bit is an engine change.
- **RPC direction is fixed.** A client's RPC goes to the server only; the server's RPC goes to all
  clients.
- **Two client windows on one machine share one keyboard.** Only the focused window receives key
  events, so you cannot drive two local clients at once by hand — use two machines, or the automated
  input hook (`RTYPE_TEST_INPUT`, see
  [Engine → Diagnostics](../engine/networking.md#11-diagnostics)).

### Engine capabilities (TODOs)

- **No prefab instantiation from Lua.** Lua can only `Entity.create()` a bare entity; it cannot
  instantiate a `.prefab`. Players are spawned by the **engine**, not by Lua (see
  [Engine → §14](../engine/networking.md#14-todos-and-planned-work)). Workaround: build the object on
  the server, then pass a `prefabPath` to `Net.replicate` so *clients* instantiate the authored prefab.
- **The 5-player cap is a compile-time constant.** `PlayerSlotState::kMaxPlayers` is `5` and the
  transport refuses a 6th connection. Supporting a 6th player needs code changes (raise the constant
  and provide enough color/sprite blocks); making it a per-project setting is a
  [TODO](../engine/networking.md#14-todos-and-planned-work).

---

## 9. Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| Clicking **Play Multiplayer** does nothing | The game executable is missing or stale — click **🔨 Build** first. |
| Client windows open but show a frozen scene | The server failed to bind `:3000` (already in use) or the client could not connect. Check the console logs. |
| Only one player appears | The scene entity is not `Client` authority, or its **Prefab Path** is empty (the server has nothing to spawn). |
| All players look identical | The color slot is not being read in your player script — use `Net.playerSlot(entity)` (see `player.lua`). |
| Changes to scripts don't show up | Rebuild the game, then relaunch; there is no hot reload. |
| Need to see traffic | Launch a process with `RTYPE_NET_DEBUG=1` and watch its log for per-second input/position lines. |
| A process lingers after Stop | Stop kills each process group; if a child was launched outside it, kill it manually. |

For the full protocol, packet reference and the `Net` API, see
[Engine → Networking & Multiplayer](../engine/networking.md).
>>>>>>> 045757470c9655b27dc515d25b99290fcc3c7993
