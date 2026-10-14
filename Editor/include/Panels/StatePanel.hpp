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

/**
 * @file StatePanel.hpp
 * @brief Declares the editor state machine editor panel and its data model.
 */
namespace Editor {

/**
 * @struct StatePanelSelectedState
 * @brief Registry resource storing the currently selected state entry.
 */
struct StatePanelSelectedState {
    /** @brief Selected state group index, or -1. */
    int groupIndex = -1;

    /** @brief Selected state index, or -1. */
    int stateIndex = -1;
};

/**
 * @struct StateBody
 * @brief Describes one state and the systems active inside it.
 */
struct StateBody {
    /** @brief State name. */
    std::string name;

    /** @brief Names of systems enabled in this state. */
    std::vector<std::string> systems;
};

/**
 * @struct StateGroup
 * @brief Describes a state machine group and all of its states.
 */
struct StateGroup {
    /** @brief Group name. */
    std::string name = "GameState";

    /** @brief State entered when the group starts. */
    std::string initialState;

    /** @brief States belonging to this group. */
    std::vector<StateBody> states;
};

/**
 * @struct StateManager
 * @brief Root data structure holding every state group.
 */
struct StateManager {
    /** @brief All configured state groups. */
    std::vector<StateGroup> groups;
};

/**
 * @class StatePanel
 * @brief Lets users author state groups and their systems in the editor.
 */
class StatePanel : public IEditorPanel {
  public:
    StatePanel() = default;
    ~StatePanel() override = default;

    /**
     * @brief Draws the state editor window.
     */
    void OnImGuiRender(Engine::Registry& registry, EditorConfig& config) override;

    /**
     * @brief Returns the panel name used for docking.
     */
    const std::string& GetName() const override;

  private:
    std::string m_Name = "State";
};

} // namespace Editor

#endif // G_ING_500_BAR_5_1_RTYPE_1_STATEPANEL_HPP