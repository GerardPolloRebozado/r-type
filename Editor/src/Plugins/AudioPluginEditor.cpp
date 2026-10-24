#include "Plugins/AudioPluginEditor.hpp"

#include "Assets/AssetManager.hpp"
#include "Components/AudioSource.hpp"
#include "EditorConfig.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <cstring>
#include <extras/IconsFontAwesome6.h>
#include <imgui.h>
#include <string>

namespace Editor {
namespace {

void LoadAudioForSource(Engine::Registry& registry, Engine::AudioSource& audio,
                        const std::string& inputPath) {
    auto stopCurrent = [&]() {
        audio.shouldPlay = false;
        audio.shouldStop = true;
        audio.isPlaying = false;
        if (!audio.assetId.empty() && registry.HasResource<Engine::AssetManager>()) {
            auto& assetManager = registry.GetResource<Engine::AssetManager>();
            try {
                assetManager.GetMusic(audio.assetId)->Stop();
            } catch (...) {
            }
        }
    };

    if (inputPath.empty()) {
        stopCurrent();
        audio.assetId.clear();
        return;
    }

    const std::string relativePath = ToProjectRelativePath(registry, inputPath);
    stopCurrent();
    audio.assetId = relativePath;
    audio.isPlaying = false;

    if (!registry.HasResource<Engine::AssetManager>()) {
        return;
    }

    auto& assetManager = registry.GetResource<Engine::AssetManager>();
    const auto resolvedPath = ResolveAssetPath(registry, audio.assetId);

    try {
        if (audio.isMusic) {
            try {
                assetManager.GetMusic(audio.assetId);
            } catch (...) {
                assetManager.LoadMusic(audio.assetId, resolvedPath.string(), audio.loop);
            }
        } else {
            try {
                assetManager.GetSound(audio.assetId);
            } catch (...) {
                assetManager.LoadSound(audio.assetId, resolvedPath.string(), audio.loop);
            }
        }
    } catch (const std::exception& error) {
        log_error("[AudioPluginEditor] Failed to load " + resolvedPath.string() + ": " +
                  error.what());
    } catch (...) {
        log_error("[AudioPluginEditor] Failed to load " + resolvedPath.string());
    }
}

} // namespace

const char* AudioPluginEditor::GetName() const {
    return "Audio Source";
}

const char* AudioPluginEditor::GetIcon() const {
    return ICON_FA_MUSIC;
}

bool AudioPluginEditor::OnImGuiRender(Engine::Entity entity, Engine::Registry& registry) {
    if (!registry.HasComponent<Engine::AudioSource>(entity)) {
        return false;
    }

    auto& audio = registry.GetComponent<Engine::AudioSource>(entity);

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
        char buffer[512];
        std::strncpy(buffer, audio.assetId.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        ImGui::Text("Asset");
        ImGui::SameLine();
        if (ImGui::InputText("##AudioAsset", buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit()) {
            LoadAudioForSource(registry, audio, buffer);
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                LoadAudioForSource(registry, audio, static_cast<const char*>(payload->Data));
            }
            ImGui::EndDragDropTarget();
        }

        bool isMusic = audio.isMusic;
        if (ImGui::Checkbox("Music", &isMusic)) {
            audio.isMusic = isMusic;
            LoadAudioForSource(registry, audio, audio.assetId);
        }
        ImGui::SameLine();
        bool loop = audio.loop;
        if (ImGui::Checkbox("Loop", &loop)) {
            audio.loop = loop;
        }

        if (ImGui::SliderFloat("Volume", &audio.volume, 0.0f, 1.0f, "%.2f")) {
            audio.volume = std::clamp(audio.volume, 0.0f, 1.0f);
        }

        if (ImGui::Button("Play")) {
            audio.shouldStop = false;
            audio.shouldPlay = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            audio.shouldPlay = false;
            audio.shouldStop = true;
        }
        ImGui::SameLine();
        if (audio.isPlaying) {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "Playing");
        } else {
            ImGui::TextDisabled("Stopped");
        }
    }

    if (remove) {
        if (registry.HasResource<Engine::AssetManager>()) {
            auto& assetManager = registry.GetResource<Engine::AssetManager>();
            try {
                assetManager.GetMusic(audio.assetId)->Stop();
            } catch (...) {
            }
        }
        RemoveComponent(entity, registry);
        return false;
    }

    return true;
}

bool AudioPluginEditor::HasComponent(Engine::Entity entity, Engine::Registry& registry) {
    return registry.HasComponent<Engine::AudioSource>(entity);
}

void AudioPluginEditor::AddComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.AddComponent<Engine::AudioSource>(entity, Engine::AudioSource());
}

void AudioPluginEditor::RemoveComponent(Engine::Entity entity, Engine::Registry& registry) {
    registry.RemoveComponent<Engine::AudioSource>(entity);
}

} // namespace Editor
