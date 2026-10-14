# Step-by-Step Tutorial

Welcome to the R-Type Engine tutorial! This guide shows how to use the editor to create your game.

## 1. Starting the Editor

Run the editor executable provided with the engine:

```bash
./editor
```

The editor startup window will appear.

## 2. Creating a New Project

1. In the **Welcome** window, click **"New Project"**
2. Enter a project name (e.g., "myFirstGame")
3. Choose a folder location for your project
4. Click **"Create"**

The editor will initialize with a fresh project.

## 3. Understanding the Editor Interface

The editor features several panels:

- **Scene Viewport:** Central area where you view and interact with your game world
- **Hierarchy Panel (left):** Lists all entities in your current scene
- **Inspector Panel (right):** Shows and editing components for the selected entity
- **Tools Panel (top):** Tools for moving, rotating, and scaling entities
- **Outliner:** Overview of project structure

## 4. Adding Your First Entity

1. In the **Hierarchy Panel**, click the **"+ Add Entity"** button
2. An entity named "Entity" is created and selected
3. The **Inspector Panel** updates to show the entity's components

## 5. Adding Components

To add a component to your entity:

1. Select your entity in the **Hierarchy Panel**
2. In the **Inspector Panel**, click **"Add Component"**
3. Choose from the available components:
   - **Position** - Sets the entity's world position (x, y)
   - **Rotation** - Sets the entity's rotation in degrees
   - **Sprite** - Adds a sprite renderer (drag a texture asset here)
   - **Collider** - Adds collision detection
   - **Script** - Attach a Lua script
   - **Camera** - Adds a camera component
   - **Parent** - Makes this entity a child of another

## 6. Using the Sprite Component

After adding a **Sprite** component:

1. Drag a texture asset from the **Assets Panel** onto the **Sprite** component's "Path" field
2. Adjust **Offset X/Y** for precise positioning
3. Adjust **Scale X/Y** to resize the sprite
4. Set **Rotation** if needed
5. Check **Visible** to enable rendering

## 6. Adding Lua Scripting

1. In the **Inspector Panel**, click **"Add Component"** → **"Script"**
2. In the Script Component settings:
   - Set **Script Path** to `scripts/player.lua` (or create this file)
   - Ensure **Auto Reload** is enabled for development
3. Create the Lua script file in your project folder:

```lua
-- scripts/player.lua
-- Moves entity with WASD

local SPEED = 200.0

function entity:onCreate()
    log.info("Script attached to entity")
end

function entity:onUpdate(dt)
    local dx, dy = 0.0, 0.0

    if Input.isKeyDown(KEY.W) then dy = dy - 1.0 end
    if Input.isKeyDown(KEY.S) then dy = dy + 1.0 end
    if Input.isKeyDown(KEY.A) then dx = dx - 1.0 end
    if Input.isKeyDown(KEY.D) then dx = dx + 1.0 end

    if dx ~= 0.0 or dy ~= 0.0 then
        local length = math.sqrt(dx * dx + dy * dy)
        dx = (dx / length) * SPEED
        dy = (dy / length) * SPEED
        Transform.translate(entity, dx * dt, dy * dt)
    end
end
```

4. The script will automatically attach to your selected entity.

## 7. Running Your Game

1. Click the **"Play"** button in the editor toolbar (or press F5)
2. The game window opens with your entities and scripts active
3. Use WASD keys to move any entity with the Lua script attached
4. Press **Esc** or click **"Stop"** to exit play mode

## 7. Saving and Exporting

1. Click **"File"** → **"Save Project"** to save your work
2. To create a standalone game executable:
   - Click **"File"** → **"Build Game"**
   - Choose the output directory
   - The editor generates a standalone game executable
   - The executable can run without the editor

## 7. Next Steps

- Add more entities and experiment with different components
- Create additional Lua scripts for different behaviors
- Import more assets (textures, sounds, etc.)
- Read the Lua API Reference for available functions
- Check the Editor Manual for advanced features

---

Need help? The editor includes contextual help tooltips, and the Community Forum is available for questions.
