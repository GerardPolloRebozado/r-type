# Entities

An entity is a numeric id. It has no data of its own; components attached to it give it
behaviour and appearance.

## Creating

- Right-click empty space in the **Hierarchy** and choose **Create Empty Entity**.
- Entities created from code use `registry.CreateEntity()`, which attaches a default
  `Position` and `Rotation`.

Every entity the editor creates gets a `Name` component so it can be displayed.

## Selecting and editing

Click an entity in the Hierarchy or in the Scene view to select it. The Inspector then shows
its components. Drag values to change them; changes apply immediately and are written to the
registry.

## Hierarchy

Drag a row onto another row to parent it; drop on empty space to detach. Parent motion is
propagated by `HierarchySystem` before `PositionSystem` snapshots previous positions, so
children follow their parents without drift.

Destroying a parent destroys its descendants. `Parent::relative = true` makes a child follow
the parent's delta rather than copying its absolute transform.

## Copy, paste, and delete

- ++ctrl+c++ / ++ctrl+v++ (Hierarchy focused) copies and pastes the selected subtree.
- Right-click a row for the same actions.
- ++delete++ removes the selected entity and its descendants.

Pasted entities keep the original parent and position, so they overlap until you move them.
Names are deduplicated.

## Naming

The `Name` component is what the editor displays. Entities without one fall back to their id.
Rename from the Inspector or by double-clicking the row in the Hierarchy.

## See also

- [Interface](interface.md)
- [Prefabs](prefabs.md)
- [Scenes](scenes.md)
