#include "Plugins/SpritePluginSerializer.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/SpriteComponent.hpp"
#include "PathUtils.hpp"
#include <iostream>

namespace Serialization {

void SpritePluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                       nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        const auto& sprite = registry.GetComponent<Engine::SpriteComponent>(entity);
        std::string relPath = ToProjectRelativePath(registry, sprite.assetPath);
        outEntityJson["SpriteComponent"] = {
            {"path", relPath},         {"offsetX", sprite.offsetX}, {"offsetY", sprite.offsetY},
            {"scaleX", sprite.scaleX}, {"scaleY", sprite.scaleY},
        };
    }
}

void SpritePluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                         const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("SpriteComponent")) {
        const auto& spriteJson = inEntityJson["SpriteComponent"];
        std::string path = spriteJson.value("path", "");
        float offsetX = spriteJson.value("offsetX", 0.0f);
        float offsetY = spriteJson.value("offsetY", 0.0f);
        float scaleX = spriteJson.value("scaleX", 1.0f);
        float scaleY = spriteJson.value("scaleY", 1.0f);

        std::string relPath = ToProjectRelativePath(registry, path);
        std::shared_ptr<Engine::Sprite> spr = nullptr;
        if (!relPath.empty()) {
            auto resolved = ResolveAssetPath(registry, relPath);
            try {
                if (registry.HasResource<Engine::AssetManager>()) {
                    auto& am = registry.GetResource<Engine::AssetManager>();
                    try {
                        auto existing = am.GetSprite(relPath);
                        if (existing && existing->GetTexture().id == 0) {
                            am.UnloadSprite(relPath);
                        }
                    } catch (...) {
                    }
                    am.LoadSprite(relPath, resolved.string());
                    spr = am.GetSprite(relPath);
                } else {
                    spr = std::make_shared<Engine::Sprite>(resolved.string());
                }
                if (spr && scaleX > 0.0f && scaleY > 0.0f) {
                    spr->SetScale({scaleX, scaleY});
                }
            } catch (const std::exception& e) {
                std::cerr << "[SpritePluginSerializer] Failed to deserialize sprite: "
                          << resolved.string() << " - Error: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[SpritePluginSerializer] Failed to deserialize sprite: "
                          << resolved.string() << " (Unknown Error)\n";
            }
        }
        registry.AddComponent<Engine::SpriteComponent>(
            entity, Engine::SpriteComponent(spr, relPath, offsetX, offsetY, scaleX, scaleY));
    }
}

} // namespace Serialization
