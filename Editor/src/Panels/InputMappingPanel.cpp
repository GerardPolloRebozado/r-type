#include "Panels/InputMappingPanel.hpp"
#include "EditorConfig.hpp"
#include <cstring>
#include <imgui.h>
#include <raylib.h>

namespace Editor {

bool InputMappingPanel::s_IsOpen = false;

InputMappingPanel::InputMappingPanel() : m_Name(ICON_FA_GAMEPAD " Input Map") {
    std::memset(m_NewActionBuffer, 0, sizeof(m_NewActionBuffer));
}

const std::string& InputMappingPanel::GetName() const {
    return m_Name;
}

const char* InputMappingPanel::GetKeyName(int key, Engine::DeviceType type) const {
    if (key == 0) {
        return "None";
    }

    static char buf[32];

    if (type == Engine::MOUSE) {
        switch (key) {
            case MOUSE_BUTTON_LEFT:
                return "Mouse Left";
            case MOUSE_BUTTON_RIGHT:
                return "Mouse Right";
            case MOUSE_BUTTON_MIDDLE:
                return "Mouse Middle";
            default:
                snprintf(buf, sizeof(buf), "Mouse %d", key);
                return buf;
        }
    }

    if (type == Engine::GAMEPAD) {
        switch (key) {
            case GAMEPAD_BUTTON_UNKNOWN:
                return "GP Unknown";
            case GAMEPAD_BUTTON_LEFT_FACE_UP:
                return "GP D-Up";
            case GAMEPAD_BUTTON_LEFT_FACE_RIGHT:
                return "GP D-Right";
            case GAMEPAD_BUTTON_LEFT_FACE_DOWN:
                return "GP D-Down";
            case GAMEPAD_BUTTON_LEFT_FACE_LEFT:
                return "GP D-Left";
            case GAMEPAD_BUTTON_RIGHT_FACE_UP:
                return "GP Y/Triangle";
            case GAMEPAD_BUTTON_RIGHT_FACE_RIGHT:
                return "GP B/Circle";
            case GAMEPAD_BUTTON_RIGHT_FACE_DOWN:
                return "GP A/Cross";
            case GAMEPAD_BUTTON_RIGHT_FACE_LEFT:
                return "GP X/Square";
            case GAMEPAD_BUTTON_LEFT_TRIGGER_1:
                return "GP L1";
            case GAMEPAD_BUTTON_LEFT_TRIGGER_2:
                return "GP L2";
            case GAMEPAD_BUTTON_RIGHT_TRIGGER_1:
                return "GP R1";
            case GAMEPAD_BUTTON_RIGHT_TRIGGER_2:
                return "GP R2";
            case GAMEPAD_BUTTON_MIDDLE_LEFT:
                return "GP Select";
            case GAMEPAD_BUTTON_MIDDLE:
                return "GP Middle";
            case GAMEPAD_BUTTON_MIDDLE_RIGHT:
                return "GP Start";
            case GAMEPAD_BUTTON_LEFT_THUMB:
                return "GP L3";
            case GAMEPAD_BUTTON_RIGHT_THUMB:
                return "GP R3";
            default:
                snprintf(buf, sizeof(buf), "GP %d", key);
                return buf;
        }
    }

    // KEYBOARD
    if (key >= 32 && key <= 126) {
        snprintf(buf, sizeof(buf), "%c", (char)key);
        return buf;
    }

    switch (key) {
        case KEY_ENTER:
            return "Enter";
        case KEY_ESCAPE:
            return "Esc";
        case KEY_UP:
            return "Up";
        case KEY_DOWN:
            return "Down";
        case KEY_LEFT:
            return "Left";
        case KEY_RIGHT:
            return "Right";
        case KEY_LEFT_SHIFT:
            return "LShift";
        case KEY_RIGHT_SHIFT:
            return "RShift";
        case KEY_LEFT_CONTROL:
            return "LCtrl";
        case KEY_RIGHT_CONTROL:
            return "RCtrl";
        case KEY_LEFT_ALT:
            return "LAlt";
        case KEY_RIGHT_ALT:
            return "RAlt";
        case KEY_TAB:
            return "Tab";
        case KEY_BACKSPACE:
            return "Backspace";
        case KEY_INSERT:
            return "Insert";
        case KEY_DELETE:
            return "Delete";
        case KEY_PAGE_UP:
            return "PgUp";
        case KEY_PAGE_DOWN:
            return "PgDn";
        case KEY_HOME:
            return "Home";
        case KEY_END:
            return "End";
        case KEY_CAPS_LOCK:
            return "CapsLock";
        case KEY_SCROLL_LOCK:
            return "ScrollLock";
        case KEY_NUM_LOCK:
            return "NumLock";
        case KEY_PRINT_SCREEN:
            return "PrintScreen";
        case KEY_PAUSE:
            return "Pause";
        case KEY_F1:
            return "F1";
        case KEY_F2:
            return "F2";
        case KEY_F3:
            return "F3";
        case KEY_F4:
            return "F4";
        case KEY_F5:
            return "F5";
        case KEY_F6:
            return "F6";
        case KEY_F7:
            return "F7";
        case KEY_F8:
            return "F8";
        case KEY_F9:
            return "F9";
        case KEY_F10:
            return "F10";
        case KEY_F11:
            return "F11";
        case KEY_F12:
            return "F12";
    }

    snprintf(buf, sizeof(buf), "Key %d", key);
    return buf;
}

void InputMappingPanel::OnImGuiRender(Engine::Registry& registry, EditorConfig& config) {
    if (!s_IsOpen) {
        return;
    }

    (void)registry;
    ImGui::Begin(m_Name.c_str(), &s_IsOpen);

    // Add New Action
    ImGui::InputText("##NewAction", m_NewActionBuffer, sizeof(m_NewActionBuffer));
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLUS " Add Action")) {
        std::string action(m_NewActionBuffer);
        if (!action.empty() && config.m_InputMap.find(action) == config.m_InputMap.end()) {
            config.m_InputMap[action] = std::vector<Engine::Keybind>();
        }
        std::memset(m_NewActionBuffer, 0, sizeof(m_NewActionBuffer));
    }

    ImGui::Separator();

    std::string actionToDelete = "";

    // List Actions
    for (auto& [actionName, keybinds] : config.m_InputMap) {
        ImGui::PushID(actionName.c_str());
        bool actionOpen = ImGui::TreeNodeEx(actionName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

        ImGui::SameLine(ImGui::GetWindowWidth() - 40);
        if (ImGui::Button(ICON_FA_TRASH)) {
            actionToDelete = actionName;
        }

        if (actionOpen) {
            int bindIndex = 0;
            int bindToDelete = -1;

            if (ImGui::BeginTable("##table", 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_Resizable)) {
                ImGui::TableSetupColumn("Device");
                ImGui::TableSetupColumn("Key");
                ImGui::TableSetupColumn("Modifier");
                ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableHeadersRow();

                for (auto& kb : keybinds) {
                    ImGui::TableNextRow();
                    ImGui::PushID(bindIndex);

                    // Device Type
                    ImGui::TableSetColumnIndex(0);
                    const char* devices[] = {"Keyboard", "Mouse", "Gamepad"};
                    int currentDevice = kb.type;
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    if (ImGui::Combo("##device", &currentDevice, devices, 3)) {
                        kb.type = static_cast<Engine::DeviceType>(currentDevice);
                    }

                    // Key
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID("KeyCol");
                    if (m_IsListeningForKey && m_ListeningAction == actionName &&
                        m_ListeningKeybindIndex == bindIndex && !m_ListeningIsModifier) {
                        ImGui::Button("Press key...", ImVec2(-FLT_MIN, 0));
                        int pressedKey = GetKeyPressed();
                        if (pressedKey != 0) {
                            kb.key = pressedKey;
                            m_IsListeningForKey = false;
                        }
                    } else {
                        if (ImGui::Button(GetKeyName(kb.key, kb.type), ImVec2(-FLT_MIN, 0))) {
                            m_IsListeningForKey = true;
                            m_ListeningAction = actionName;
                            m_ListeningKeybindIndex = bindIndex;
                            m_ListeningIsModifier = false;
                        }
                    }
                    ImGui::PopID();

                    // Modifier (ComposeKey)
                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushID("ModCol");
                    if (m_IsListeningForKey && m_ListeningAction == actionName &&
                        m_ListeningKeybindIndex == bindIndex && m_ListeningIsModifier) {
                        ImGui::Button("Press key...", ImVec2(-FLT_MIN, 0));
                        int pressedKey = GetKeyPressed();
                        if (pressedKey != 0) {
                            kb.composeKey = pressedKey;
                            m_IsListeningForKey = false;
                        }
                    } else {
                        // Modifiers are typically always keyboard keys in this engine right now
                        if (ImGui::Button(GetKeyName(kb.composeKey, Engine::KEYBOARD),
                                          ImVec2(-FLT_MIN, 0))) {
                            m_IsListeningForKey = true;
                            m_ListeningAction = actionName;
                            m_ListeningKeybindIndex = bindIndex;
                            m_ListeningIsModifier = true;
                        }
                    }
                    ImGui::PopID();

                    // Delete Bind
                    ImGui::TableSetColumnIndex(3);
                    if (ImGui::Button(ICON_FA_XMARK)) {
                        bindToDelete = bindIndex;
                    }

                    ImGui::PopID();
                    bindIndex++;
                }
                ImGui::EndTable();
            }

            if (bindToDelete != -1) {
                keybinds.erase(keybinds.begin() + bindToDelete);
            }

            if (ImGui::Button(ICON_FA_PLUS " Add Keybind")) {
                keybinds.push_back({0, 0, Engine::KEYBOARD});
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::Separator();
    }

    if (!actionToDelete.empty()) {
        config.m_InputMap.erase(actionToDelete);
    }

    if (m_IsListeningForKey && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) {
        m_IsListeningForKey = false;
    }

    ImGui::End();
}

} // namespace Editor
