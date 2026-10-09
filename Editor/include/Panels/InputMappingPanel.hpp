#ifndef EDITOR_PANELS_INPUTMAPPINGPANEL_HPP_
#define EDITOR_PANELS_INPUTMAPPINGPANEL_HPP_

#include "Inputs/IController.hpp"
#include "Panels/IEditorPanel.hpp"
#include <extras/IconsFontAwesome6.h>
#include <string>

namespace Editor {

class InputMappingPanel : public IEditorPanel {
  public:
    InputMappingPanel();
    ~InputMappingPanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

    static bool s_IsOpen;

  private:
    std::string m_Name;
    char m_NewActionBuffer[256];

    bool m_IsListeningForKey = false;
    std::string m_ListeningAction = "";
    int m_ListeningKeybindIndex = -1;
    bool m_ListeningIsModifier = false;

    const char* GetKeyName(int key, Engine::DeviceType type) const;
};

} // namespace Editor
#endif // EDITOR_PANELS_INPUTMAPPINGPANEL_HPP_
