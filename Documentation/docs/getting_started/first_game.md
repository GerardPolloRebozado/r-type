# Your First Game

This section introduces the R-Type Engine workflow for creating games using the editor and Lua scripting.

## Overview

The R-Type Engine includes a full-featured editor executable that lets you create games without writing C++ code. The editor provides:
- A visual interface for creating entities and adding components
- A Lua scripting system for gameplay logic
- A scene editor for designing game levels
- Tools for building and exporting standalone games

## Typical Workflow

1. **Run the editor executable** provided with the engine
2. **Create a new project** using the welcome screen
3. **Add entities** via the Hierarchy panel
4. **Add components** via the Inspector panel (Position, Sprite, Collider, Script)
5. **Write Lua scripts** for gameplay logic
6. **Press Play** to test your game in the editor
7. **Build a standalone executable** when ready

## Lua Scripting

The engine includes a built-in Lua scripting system. Scripts attach to entities and run lifecycle functions:
- `onCreate()` - Called once when the script is loaded
- `onUpdate(dt)` - Called every frame with the delta time
- `onDestroy()` - Called when the script/entity is removed

Scripts can:
- Move entities using Transform functions
- Play and manage sounds and music
- Detect collisions and triggers
- Communicate with other entities
- Load and manage assets

## Building Your Game

When your game is ready:
1. Use the editor's **Build Game** feature
2. A standalone executable is generated
3. The executable runs without the editor
4. Distribute the executable as needed

## Learning Resources

- The editor includes contextual help tooltips
- The Lua API Reference documents all available functions
- The Editor Manual covers advanced editor features
- Sample projects are included with the engine distribution