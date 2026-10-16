#ifndef EDITOR_PANELS_ASSETSPANEL_HPP_
#define EDITOR_PANELS_ASSETSPANEL_HPP_

#include "Core/Application.hpp"
#include "Panels/IEditorPanel.hpp"
#include <filesystem>
#include <string>

namespace Editor {

class AssetsPanel : public IEditorPanel {
  public:
    AssetsPanel() = default;
    ~AssetsPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    void DrawDialogs(Engine::Registry& registry, EditorConfig& config);
    bool Relocate(Engine::Registry& registry, EditorConfig& config,
                  const std::filesystem::path& source, const std::filesystem::path& destination);

    std::string m_Name = "Assets";
    std::filesystem::path m_CurrentDirectory;
    std::filesystem::path m_StartingDirectory;
    std::filesystem::path m_LastProjectDirectory;
    std::string m_DialogText;
    std::filesystem::path m_DialogTarget;
    bool m_OpenNewFolder = false;
};

} // namespace Editor
#endif // EDITOR_PANELS_ASSETSPANEL_HPP_
