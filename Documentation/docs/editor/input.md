# Input mapping

The **Input Map** panel edits the actions and keybinds stored in `input.json`. Actions are
string names that gameplay reads through `Controller::isActionActive`.

## Using the panel

1. Open **Input Map** from the menu bar or its tab.
2. Type an action name and click **+ Add Action**.
3. Expand the action, click **+ Add Keybind**, then click the **Key** button and press the
   desired key.

Action names are case-sensitive: `left`, `Left`, and `LEFT` are three different actions.

## Storage

The map is stored in the scene as `m_InputMap` and baked into the game launcher at build
time. Changing it requires a rebuild before the game sees the new bindings.

## See also

- [Engine → Input](../engine/input.md)
- [Play mode](play.md)
