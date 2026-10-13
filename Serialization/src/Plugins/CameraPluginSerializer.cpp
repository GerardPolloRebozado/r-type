#include "Plugins/CameraPluginSerializer.hpp"
#include "Components/CameraComponent.hpp"
#include "Plugins/CameraPlugin.hpp"

namespace Serialization {

void CameraPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                       nlohmann::json& outEntityJson) {
    // Projects that never init CameraPlugin must still be able to save
    if (!registry.IsComponentRegistered<Engine::CameraComponent>() ||
        !registry.HasComponent<Engine::CameraComponent>(entity)) {
        return;
    }
    const auto& camera = registry.GetComponent<Engine::CameraComponent>(entity);
    outEntityJson["CameraComponent"] = {
        {"zoom", camera.zoom},
        {"primary", camera.primary},
        {"followTarget", camera.followTarget ? static_cast<int>(*camera.followTarget) : -1},
        {"followSpeed", camera.followSpeed},
        {"clampToBounds", camera.clampToBounds},
        {"boundsX", camera.bounds.x},
        {"boundsY", camera.bounds.y},
        {"boundsWidth", camera.bounds.width},
        {"boundsHeight", camera.bounds.height},
        {"shakeMagnitude", camera.shakeMagnitude},
        {"shakeDecay", camera.shakeDecay},
    };
}

void CameraPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                         const nlohmann::json& inEntityJson) {
    if (!inEntityJson.contains("CameraComponent")) {
        return;
    }
    const auto& json = inEntityJson["CameraComponent"];
    Engine::CameraComponent camera(json.value("zoom", 1.0f), json.value("primary", true));
    const int target = json.value("followTarget", -1);
    if (target >= 0) {
        camera.followTarget = static_cast<Engine::Entity>(target);
    }
    camera.followSpeed = json.value("followSpeed", 0.0f);
    camera.clampToBounds = json.value("clampToBounds", false);
    camera.bounds = {json.value("boundsX", 0.0f), json.value("boundsY", 0.0f),
                     json.value("boundsWidth", 0.0f), json.value("boundsHeight", 0.0f)};
    camera.shakeMagnitude = json.value("shakeMagnitude", 10.0f);
    camera.shakeDecay = json.value("shakeDecay", 1.5f);
    Engine::CameraPlugin::AttachToEntity(registry, entity, camera);
}

} // namespace Serialization
