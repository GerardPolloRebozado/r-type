# ImGuiColorTextEdit

Vendored copy of BalazsJako/ImGuiColorTextEdit for the Lua script editor.

- Upstream: https://github.com/BalazsJako/ImGuiColorTextEdit
- Pinned commit: `ca2f9f1462e3b60e56351bc466acda448c5ea50d`
- License: MIT (see `LICENSE`)

Local changes:

- Added `TextEditor::ReplaceSelection()` so find/replace is a single undoable edit.
- Added `TextEditor::SetTextStart()` so the Lua editor can reserve room for line numbers.

Please, keep local modifications small and documented here so the widget can be updated
without losing project-specific changes.
