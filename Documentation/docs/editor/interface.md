# Editor Interface Overview

The Editor is built using `rlImGui`.

## Scene View
Displays the live game world at the project's virtual resolution, scaled to fit the panel.

## Hierarchy Panel
Lists all active entities in the `Registry`. You can right-click to spawn new entities.

## Inspector Panel
Shows the components attached to the selected entity. Modify values here in real-time.

## Copy, paste, and prefabs
Right-click a Hierarchy row, or press Ctrl+C while the Hierarchy panel is focused, to copy that entity and its children. Ctrl+V, or Paste in the context menu, creates a new subtree with the same data. The copy keeps the original parent and the same position, so it overlaps the original until you move it. A name that is already used becomes `enemy1(1)`, then `enemy1(2)`, and the same numbering is applied to a spawned prefab.

Drag a Hierarchy row onto the Assets panel to write `<Name>.prefab` in the folder you are viewing. The name comes from the Name component. If that file already exists, the next file is `Name_2.prefab`, then `Name_3.prefab`. Drag the `.prefab` back onto the Hierarchy background, or onto a row, to spawn that subtree.

The spawned root, and the entity you dragged out, store the prefab path. Selecting that `.prefab` in Assets opens its components in the Preview panel. Changing the sprite path, sprite scale, shape size, or script there updates every instance of that prefab. Positions stay where each instance was placed.

## Console
Shows problems in the content you control: a script error, a prefab or scene that failed to load, a rejected name or path, or a hierarchy attach the editor refused. Engine and editor diagnostics are not listed here. An empty panel means there is no such problem. `print`, `log.info`, and `log.debug` from a script stay on the terminal.
