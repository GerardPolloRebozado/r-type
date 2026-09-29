#pragma once

#include "Panels/IEditorPanel.hpp"
#include <string>

namespace Editor {

class SceneViewPanel : public IEditorPanel {
  public:
    SceneViewPanel() = default;
    ~SceneViewPanel() override = default;

    void OnImGuiRender() override;
    const std::string& GetName() const override {
        return m_Name;
    }

  private:
    std::string m_Name = "Scene";
};

} // namespace Editor
