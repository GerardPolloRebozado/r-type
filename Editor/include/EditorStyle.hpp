#ifndef EDITOR_EDITORSTYLE_HPP_
#define EDITOR_EDITORSTYLE_HPP_

/**
 * @file EditorStyle.hpp
 * @brief Declares the ImGui style used by the editor.
 */

namespace Editor {

/**
 * @class EditorStyle
 * @brief Static entry point that applies the editor's dark theme.
 */
class EditorStyle {
  public:
    /**
     * @brief Applies the editor colors, spacing and rounding to ImGui.
     */
    static void Apply();
};

} // namespace Editor

#endif // EDITOR_EDITORSTYLE_HPP_