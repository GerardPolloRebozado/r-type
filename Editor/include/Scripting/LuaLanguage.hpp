#ifndef EDITOR_SCRIPTING_LUALANGUAGE_HPP_
#define EDITOR_SCRIPTING_LUALANGUAGE_HPP_

#include "TextEditor.h"

namespace Editor {

/**
 * @brief Lua language definition used by the script editor.
 *
 * The base definition comes from ImGuiColorTextEdit. This wrapper enables
 * auto-indentation and adds the engine namespaces so they appear in the
 * editor's identifier hover tooltips.
 */
TextEditor::LanguageDefinition MakeLuaLanguageDefinition();

/**
 * @brief Dark palette aligned with the editor's blue/grey theme.
 */
TextEditor::Palette MakeLuaEditorPalette();

} // namespace Editor

#endif // EDITOR_SCRIPTING_LUALANGUAGE_HPP_
