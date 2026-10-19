#include "Plugins/NetworkComponentPluginEditor.hpp"

#include "Components/NetworkComponent.hpp"
#include "Components/Position.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>

namespace Editor {

const char* NetworkComponentPluginEditor::GetName() const {
    return "Network";
}

const char* NetworkComponentPluginEditor::GetIcon() const {
    return ICON_FA_SITEMAP;
}

bool NetworkComponentPluginEditor::OnImGuiRender(Engine::Entity entity,
                                                 Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::NetworkComponent>(entity)) {
        return false;
    }

    auto& netComp = registry.GetComponent<Engine::NetworkComponent>(entity);

    std::string header = std::string(GetIcon()) + " " + GetName();
    bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    bool remove = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            remove = true;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        // Authority dropdown
        const char* authorities[] = {"Server", "Client", "Shared"};
        int currentAuthority = 0;
        if (netComp.authority == "Server") {
            currentAuthority = 0;
        } else if (netComp.authority == "Client") {
            currentAuthority = 1;
        } else if (netComp.authority == "Shared") {
            currentAuthority = 2;
        }
        if (ImGui::Combo("Authority", &currentAuthority, authorities, IM_ARRAYSIZE(authorities))) {
            netComp.authority = authorities[currentAuthority];
        }

        // Sync checkboxes
        bool syncTransform = netComp.syncTransform;
        if (ImGui::Checkbox("Sync Transform", &syncTransform)) {
            netComp.syncTransform = syncTransform;
        }

        bool syncSprite = netComp.syncSprite;
        if (ImGui::Checkbox("Sync Sprite", &syncSprite)) {
            netComp.syncSprite = syncSprite;
        }

        // Prefab path (for spawn-from-prefab on connect)
        char prefabBuffer[256];
        strncpy(prefabBuffer, netComp.prefabPath.c_str(), sizeof(prefabBuffer));
        prefabBuffer[sizeof(prefabBuffer) - 1] = '\0';

        ImGui::Text("Prefab Path");
        ImGui::SameLine();
        if (ImGui::InputText("##PrefabPath", prefabBuffer, sizeof(prefabBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit()) {
            netComp.prefabPath = std::string(prefabBuffer);
        }

        // Drag and drop target for prefab path
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                const char* path = (const char*)payload->Data;
                std::string relPath = ToProjectRelativePath(registry, std::string(path));
                netComp.prefabPath = relPath;
            }
            ImGui::EndDragDropTarget();
        }

        // Network ID (read-only, engine-assigned)
        if (netComp.networkId != 0) {
            ImGui::Text("Network ID: %llu", netComp.networkId);
        } else {
            ImGui::TextDisabled("Network ID: (assigned on connect)");
        }
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool NetworkComponentPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::NetworkComponent>(entity);
}

void NetworkComponentPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.AddComponent<Engine::NetworkComponent>(entity, Engine::NetworkComponent{});
    if (!registry.HasComponent<Engine::Position>(entity)) {
        registry.AddComponent<Engine::Position>(entity, Engine::Position(0.0f, 0.0f, 0.0f));
    }
}

void NetworkComponentPluginEditor::RemoveComponent(Engine::Entity entity,
                                                   Engine::Registry& registry) {
    registry.RemoveComponent<Engine::NetworkComponent>(entity);
}

} // namespace Editor