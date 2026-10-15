#include "Plugins/ScriptPluginEditor.hpp"
#include "Components/Name.hpp"
#include "Components/Position.hpp"
#include "EditorConfig.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/ScriptPlugin.hpp"
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include <filesystem>
#include <fstream>
#include <imgui.h>

namespace Editor {

const char* ScriptPluginEditor::GetName() const {
    return "Script";
}

const char* ScriptPluginEditor::GetIcon() const {
    return ICON_FA_CODE;
}

bool ScriptPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::ScriptComponent>(entity)) {
        return false;
    }

    auto& script = registry.GetComponent<Engine::ScriptComponent>(entity);

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            RemoveComponent(entity, registry);
            ImGui::EndPopup();
            return false;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        if (ImGui::Button("Create New Script")) {
            ImGui::OpenPopup("CreateScriptPopup");
        }
        if (ImGui::BeginPopupModal("CreateScriptPopup", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            static char scriptName[128] = "";
            ImGui::Text("Enter script name (no extension):");
            ImGui::InputText("##ScriptName", scriptName, sizeof(scriptName));
            if (ImGui::Button("Create")) {
                std::string name(scriptName);
                if (!name.empty()) {
                    std::filesystem::path scriptsDir = "scripts";
                    std::string projectFolder;
                    if (registry.HasResource<EditorConfig>()) {
                        projectFolder = registry.GetResource<EditorConfig>().m_ProjectFolder;
                        scriptsDir = std::filesystem::path(projectFolder) / "scripts";
                    }
                    std::filesystem::create_directories(scriptsDir);

                    std::string fileName = name;
                    if (fileName.find(".lua") == std::string::npos) {
                        fileName += ".lua";
                    }
                    std::filesystem::path fullPath = scriptsDir / fileName;

                    // Write template if file doesn't exist
                    if (!std::filesystem::exists(fullPath)) {
                        std::string entityName = "Entity";
                        if (registry.HasComponent<Engine::Name>(entity)) {
                            entityName = registry.GetComponent<Engine::Name>(entity).value;
                        }
                        std::string templateContent =
                            "-- " + fileName +
                            "\n"
                            "-- Attached to: " +
                            entityName +
                            "\n\n"
                            "local Script = {}\n\n"
                            "function Script:onCreate(entity)\n"
                            "    self.entity = entity\n"
                            "    log.info(\"[\" .. tostring(entity) .. \"] Script created\")\n"
                            "end\n\n"
                            "function Script:onUpdate(entity, dt)\n"
                            "    -- Add your per-frame logic here\n"
                            "end\n\n"
                            "function Script:onDestroy(entity)\n"
                            "    log.info(\"[\" .. tostring(entity) .. \"] Script destroyed\")\n"
                            "end\n\n"
                            "return Script\n";
                        std::ofstream out(fullPath);
                        out << templateContent;
                        out.close();
                    }

                    // Set relative path from project root
                    std::string relPath = "scripts/" + fileName;
                    script.scriptPath = relPath;
                    script.loaded = false;
                    script.scriptTable = sol::lua_nil;
                    scriptName[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                scriptName[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        char buffer[512];
        std::strncpy(buffer, script.scriptPath.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Script Path");
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##ScriptPath", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            script.scriptPath = buffer;
            // Force a reload next update.
            script.loaded = false;
            script.scriptTable = sol::lua_nil;
        }
        ImGui::PopItemWidth();

        // Drag & drop a .lua file from the Assets panel.
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = static_cast<const char*>(payload->Data);
                script.scriptPath = path;
                script.loaded = false;
                script.scriptTable = sol::lua_nil;
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Checkbox("Auto Reload", &script.autoReload);

        if (!script.scriptPath.empty()) {
            bool exists = std::filesystem::exists(script.scriptPath);
            if (script.loaded) {
                ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Loaded");
            } else if (exists) {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.4f, 1.0f), "Not loaded yet");
            } else {
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "File not found");
            }
        }

        ImGui::TextDisabled("Drop a .lua file or type a path relative to the project folder.");
    }

    return true;
}

bool ScriptPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::ScriptComponent>(entity);
}

void ScriptPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    Engine::ScriptPlugin::AttachToEntity(registry, entity, Engine::ScriptComponent{});
    if (!registry.HasComponent<Engine::Position>(entity)) {
        registry.AddComponent<Engine::Position>(entity, Engine::Position(0.0f, 0.0f));
    }
}

void ScriptPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::ScriptComponent>(entity);
}

} // namespace Editor
