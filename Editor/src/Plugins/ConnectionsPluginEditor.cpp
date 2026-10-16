/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** ConnectionsPluginEditor.cpp
*/

#include "Plugins/ConnectionsPluginEditor.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Name.hpp"
#include "Components/SignalConnections.hpp"
#include "Plugins/SignalPlugin.hpp"

#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>

namespace Editor {

namespace {

std::string EntityLabel(Engine::Registry& registry, Engine::Entity entity) {
    if (registry.HasComponent<Engine::Name>(entity)) {
        const std::string& name = registry.GetComponent<Engine::Name>(entity).value;
        if (!name.empty()) {
            return name + " (" + std::to_string(entity) + ")";
        }
    }
    return "Entity " + std::to_string(entity);
}

bool BelongsToProvider(const Engine::SignalConnection& connection,
                       const Engine::Signals::SignalProvider& provider) {
    for (const auto& signal : provider.signals) {
        if (connection.signal == signal) {
            return true;
        }
    }
    return false;
}

void TargetCombo(Engine::Registry& registry, Engine::Entity& target) {
    std::vector<Engine::Entity> candidates;
    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Engine::Entity candidate{i};
        if (registry.IsEntityAlive(candidate)) {
            candidates.push_back(candidate);
        }
    }
    bool targetAlive = false;
    for (const auto& candidate : candidates) {
        if (candidate == target) {
            targetAlive = true;
            break;
        }
    }
    if (!targetAlive) {
        target = candidates.empty() ? static_cast<Engine::Entity>(-1) : candidates.front();
    }
    const std::string targetPreview =
        (target == static_cast<Engine::Entity>(-1)) ? "<none>" : EntityLabel(registry, target);
    if (ImGui::BeginCombo("Target", targetPreview.c_str())) {
        for (const auto& candidate : candidates) {
            const bool selected = (candidate == target);
            if (ImGui::Selectable(EntityLabel(registry, candidate).c_str(), selected)) {
                target = candidate;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

} // namespace

const char* ConnectionsPluginEditor::GetName() const {
    return "Connections";
}

const char* ConnectionsPluginEditor::GetIcon() const {
    return ICON_FA_LINK;
}

bool ConnectionsPluginEditor::OnImGuiRender(const Engine::Entity entity,
                                            Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::SignalConnections>(entity)) {
        return false;
    }

    const std::string header = std::string(GetIcon()) + " " + GetName();
    const bool isOpen = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    bool remove = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Remove Component")) {
            remove = true;
        }
        ImGui::EndPopup();
    }

    if (isOpen) {
        RenderTabs(entity, registry);
    }

    if (remove) {
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool ConnectionsPluginEditor::RenderTabs(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::SignalConnections>(entity)) {
        return false;
    }
    auto& stored = registry.GetComponent<Engine::SignalConnections>(entity);

    const std::vector<Engine::Signals::SignalProvider> providers =
        Engine::Signals::GetSignalProviders();
    if (providers.empty()) {
        ImGui::TextDisabled("No signal providers registered.");
        return true;
    }
    if (ImGui::BeginTabBar("ConnectionsTabs")) {
        for (size_t t = 0; t < providers.size(); ++t) {
            if (ImGui::BeginTabItem(providers[t].plugin.data())) {
                RenderProviderTab(entity, registry, stored, providers[t], static_cast<int>(t));
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    return true;
}

void ConnectionsPluginEditor::RenderProviderTab(Engine::Entity entity, Engine::Registry& registry,
                                                Engine::SignalConnections& stored,
                                                const Engine::Signals::SignalProvider& provider,
                                                int tabIndex) {
    size_t shown = 0;
    int rowToRemove = -1;
    for (size_t i = 0; i < stored.connections.size(); ++i) {
        const auto& connection = stored.connections[i];
        if (!BelongsToProvider(connection, provider)) {
            continue;
        }
        ++shown;
        const bool targetAlive = registry.IsEntityAlive(connection.target);
        if (!targetAlive) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
        }
        ImGui::BulletText("%s -> %s :: %s", connection.signal.c_str(),
                          EntityLabel(registry, connection.target).c_str(),
                          connection.method.c_str());
        if (!targetAlive) {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        const std::string removeId = "X##conn" + std::to_string(tabIndex) + "_" + std::to_string(i);
        if (ImGui::SmallButton(removeId.c_str())) {
            rowToRemove = static_cast<int>(i);
        }
    }
    if (rowToRemove >= 0) {
        stored.connections.erase(stored.connections.begin() + rowToRemove);
    }
    if (shown == 0) {
        ImGui::TextDisabled("No %s connections yet.", provider.plugin.data());
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Add connection");

    if (provider.plugin == "Collider" &&
        !registry.HasComponent<Engine::ColliderComponent>(entity)) {
        ImGui::TextDisabled("Emitter has no Collider: collision signals will not fire.");
    }

    if (provider.signals.empty()) {
        ImGui::TextDisabled("This provider declares no signals.");
        return;
    }
    if (m_SignalIdx < 0 || m_SignalIdx >= static_cast<int>(provider.signals.size())) {
        m_SignalIdx = 0;
    }
    const char* signalPreview = provider.signals[m_SignalIdx].data();
    if (ImGui::BeginCombo("Signal", signalPreview)) {
        for (size_t i = 0; i < provider.signals.size(); ++i) {
            const bool selected = (static_cast<int>(i) == m_SignalIdx);
            if (ImGui::Selectable(provider.signals[i].data(), selected)) {
                m_SignalIdx = static_cast<int>(i);
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    TargetCombo(registry, m_Target);
    ImGui::InputText("Method", &m_Method);

    const bool canAdd = !m_Method.empty() && m_Target != static_cast<Engine::Entity>(-1) &&
                        registry.IsEntityAlive(m_Target);
    if (!canAdd) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Connect")) {
        const std::string signal{provider.signals[m_SignalIdx]};
        bool duplicate = false;
        for (const auto& connection : stored.connections) {
            if (connection.signal == signal && connection.target == m_Target &&
                connection.method == m_Method) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            stored.connections.push_back(Engine::SignalConnection{signal, m_Target, m_Method});
            m_Method.clear();
        }
    }
    if (!canAdd) {
        ImGui::EndDisabled();
    }
}

bool ConnectionsPluginEditor::HasComponent(const Engine::Entity entity,
                                           Engine::Registry& registry) {
    return registry.HasComponent<Engine::SignalConnections>(entity);
}

void ConnectionsPluginEditor::AddComponent(const Engine::Entity entity,
                                           Engine::Registry& registry) {
    Engine::SignalPlugin::AttachToEntity(registry, entity);
}

void ConnectionsPluginEditor::RemoveComponent(const Engine::Entity entity,
                                              Engine::Registry& registry) {
    registry.RemoveComponent<Engine::SignalConnections>(entity);
}
} // namespace Editor
