# Adding editor features

## Add a panel

1. Create a class implementing `IEditorPanel` in `Editor/include/Panels/` and
   `Editor/src/Panels/`.
2. Register it in `EditorLayer`'s panel list and dockspace layout.
3. Add it to the default dock layout if needed.

## Add an inspector section

Implement `IPluginEditor` and register it in `InspectorPanel::InspectorPanel()`.

## Add a menu item

Add to `EditorLayer::DrawSceneMenuItems()` or the relevant menu builder. Keep shortcuts
consistent with the existing ++ctrl+s++ / ++ctrl+c++ / ++ctrl+v++ scheme.

## Guidelines

- The editor only writes component data; systems do the work.
- Clamp values early in inspector widgets.
- Keep serialization in sync.
- Use asset pickers for paths.

## See also

- [Editor architecture](architecture-editor.md)
- [Interface](../editor/interface.md)
- [Editor plugins](../plugins/editor-plugins.md)
- [Command line](../editor/cli.md)
