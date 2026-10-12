/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** StatePanel.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_STATEPANEL_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_STATEPANEL_HPP
#include "IEditorPanel.hpp"
#include <string>
#include <vector>

namespace Editor {
struct StatePanelSelectedState {
    int groupIndex = -1;
    int stateIndex = -1;
};

struct StateBody {
    std::string name;
    std::vector<std::string> systems;
};

struct StateGroup {
    std::string name = "GameState";
    std::string initialState;
    std::vector<StateBody> states;
};

struct StateManager {
    std::vector<StateGroup> groups;
};

class StatePanel : public IEditorPanel {
  public:
    StatePanel() = default;
    ~StatePanel() override = default;

    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;
    const std::string& GetName() const override;

  private:
    std::string m_Name = "State";
};

} // namespace Editor

#endif // G_ING_500_BAR_5_1_RTYPE_1_STATEPANEL_HPP
