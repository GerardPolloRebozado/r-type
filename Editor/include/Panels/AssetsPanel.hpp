#ifndef EDITOR_PANELS_ASSETSPANEL_HPP_
#define EDITOR_PANELS_ASSETSPANEL_HPP_

/**
 * @file AssetsPanel.hpp
 * @brief Declares the project asset browser panel.
 */

#include "Core/Application.hpp"
#include "Panels/IEditorPanel.hpp"
#include <filesystem>

namespace Editor {

/**
 * @class AssetsPanel
 * @brief Content browser used to browse, select and import project assets.
 */
class AssetsPanel : public IEditorPanel {
  public:
    AssetsPanel() = default;
    ~AssetsPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "Assets";
    std::filesystem::path m_CurrentDirectory;
    std::filesystem::path m_StartingDirectory;
    std::filesystem::path m_LastProjectDirectory;
};

} // namespace Editor
#endif // EDITOR_PANELS_ASSETSPANEL_HPP_
