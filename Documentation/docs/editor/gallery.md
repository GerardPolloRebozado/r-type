# Gallery

The Gallery is the first screen you see when you launch `RTypeEditor` with no arguments. It
lists your projects and lets you create new ones.

## What it shows

Each project card displays the project name, description, path, and last-opened date. Click
a card to open the project's startup scene in the editor. Projects are stored in
`projects.json` under your configuration directory.

## Creating a project

Click **+ New Project** and fill in the dialog:

| Field | Meaning |
|---|---|
| Name | Window title and project identity |
| Description | Optional text shown on the card |
| Path | Project folder; use **Browse...** to pick it |

A project folder contains `test.scene`, `project.json`, `assets/`, and `input.json`.
`main.cpp` is generated when you build.

## Opening a scene directly

```bash
./build/RTypeEditor path/to/test.scene
```

This skips the Gallery and opens the scene. The project folder is the scene's parent.

## Removing a project

Projects can be removed from the list from the card's options menu. Removing a project from
the Gallery does not delete its files.

## See also

- [Command line](cli.md)  -  `new`, `create system`, `create plugin`
- [Scenes](scenes.md)  -  startup scenes and `project.json`
