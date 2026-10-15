#include "GlazeSerializer.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <unordered_map>

namespace Serialization {
GlazeSerializer::GlazeSerializer(SerializationFormat format) : m_Format(format) {}

SerializationFormat GlazeSerializer::GetFormat() const {
    return m_Format;
}

void GlazeSerializer::SetFormat(SerializationFormat format) {
    m_Format = format;
}

GlazeSerializer::SceneSnapshot GlazeSerializer::CaptureScene(Engine::Registry& registry) {
    SceneSnapshot snapshot;
    snapshot.sceneName = "Untitled";

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = i;
        EntitySnapshot entitySnapshot;
        entitySnapshot.entityId = entity;
        bool hasAnyComponent = false;

        if (registry.HasComponent<Engine::Name>(entity)) {
            entitySnapshot.name = registry.GetComponent<Engine::Name>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Position>(entity)) {
            entitySnapshot.position = registry.GetComponent<Engine::Position>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Rotation>(entity)) {
            entitySnapshot.rotation = registry.GetComponent<Engine::Rotation>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
            entitySnapshot.sprite = registry.GetComponent<Engine::SpriteComponent>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
            entitySnapshot.shape = registry.GetComponent<Engine::ShapeComponent>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Text>(entity)) {
            entitySnapshot.text = registry.GetComponent<Engine::Text>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
            entitySnapshot.collider = registry.GetComponent<Engine::ColliderComponent>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Opacity>(entity)) {
            entitySnapshot.opacity = registry.GetComponent<Engine::Opacity>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Parent>(entity)) {
            entitySnapshot.parent = registry.GetComponent<Engine::Parent>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::Input>(entity)) {
            entitySnapshot.input = registry.GetComponent<Engine::Input>(entity);
            hasAnyComponent = true;
        }
        if (registry.HasComponent<Engine::AudioSource>(entity)) {
            entitySnapshot.audioSource = registry.GetComponent<Engine::AudioSource>(entity);
            hasAnyComponent = true;
        }

        if (registry.HasComponent<Engine::CameraComponent>(entity)) {
            entitySnapshot.camera = registry.GetComponent<Engine::CameraComponent>(entity);
            hasAnyComponent = true;
        }

        if (registry.HasComponent<Engine::ScriptComponent>(entity)) {
            entitySnapshot.script = registry.GetComponent<Engine::ScriptComponent>(entity);
            hasAnyComponent = true;
        }

        // Skip entity slots that were never created or have been destroyed so that
        // restored entity IDs stay compact and parent links can be remapped exactly.
        if (hasAnyComponent) {
            snapshot.entities.push_back(std::move(entitySnapshot));
        }
    }

    return snapshot;
}

void GlazeSerializer::RestoreScene(Engine::Registry& registry, const SceneSnapshot& snapshot,
                                   std::unordered_map<Engine::Entity, Engine::Entity>* outRemap) {
    // Pass 1: recreate entities and restore every component except Parent, recording
    // the mapping from serialized entity IDs to the freshly created ones.
    std::unordered_map<Engine::Entity, Engine::Entity> idRemap;
    idRemap.reserve(snapshot.entities.size());

    for (const auto& entitySnapshot : snapshot.entities) {
        Engine::Entity entity = registry.CreateEntity();
        idRemap[entitySnapshot.entityId] = entity;

        if (entitySnapshot.name.has_value()) {
            registry.AddComponent<Engine::Name>(entity, entitySnapshot.name.value());
        }
        if (entitySnapshot.position.has_value()) {
            // Fresh baseline: a loaded scene did not "move", so prev must
            // equal the restored position. Stale file values would inject a
            // phantom delta into relative children on the first frame.
            Engine::Position restored = entitySnapshot.position.value();
            restored.prevX = restored.x;
            restored.prevY = restored.y;
            restored.moved = false;
            registry.AddComponent<Engine::Position>(entity, restored);
        }
        if (entitySnapshot.rotation.has_value()) {
            registry.AddComponent<Engine::Rotation>(entity, entitySnapshot.rotation.value());
        }
        if (entitySnapshot.sprite.has_value()) {
            registry.AddComponent<Engine::SpriteComponent>(entity, entitySnapshot.sprite.value());
        }
        if (entitySnapshot.shape.has_value()) {
            registry.AddComponent<Engine::ShapeComponent>(entity, entitySnapshot.shape.value());
        }
        if (entitySnapshot.text.has_value()) {
            registry.AddComponent<Engine::Text>(entity, entitySnapshot.text.value());
        }
        if (entitySnapshot.collider.has_value()) {
            registry.AddComponent<Engine::ColliderComponent>(entity,
                                                             entitySnapshot.collider.value());
        }
        if (entitySnapshot.opacity.has_value()) {
            registry.AddComponent<Engine::Opacity>(entity, entitySnapshot.opacity.value());
        }
        if (entitySnapshot.input.has_value()) {
            registry.AddComponent<Engine::Input>(entity, entitySnapshot.input.value());
        }
        if (entitySnapshot.audioSource.has_value()) {
            registry.AddComponent<Engine::AudioSource>(entity, entitySnapshot.audioSource.value());
        }
        if (entitySnapshot.camera.has_value()) {
            registry.AddComponent<Engine::CameraComponent>(entity, entitySnapshot.camera.value());
        }
        if (entitySnapshot.script.has_value()) {
            registry.AddComponent<Engine::ScriptComponent>(entity, entitySnapshot.script.value());
        }
    }

    // Pass 2: restore Parent links with remapped entity IDs so hierarchies survive
    // even when serialized IDs do not match the freshly created ones.
    for (const auto& entitySnapshot : snapshot.entities) {
        if (!entitySnapshot.parent.has_value()) {
            continue;
        }
        const Engine::Entity entity = idRemap[entitySnapshot.entityId];
        const Engine::Entity oldParent = entitySnapshot.parent->parent;
        const auto it = idRemap.find(oldParent);
        const Engine::Entity newParent = (it != idRemap.end()) ? it->second : oldParent;
        registry.AddComponent<Engine::Parent>(entity, Engine::Parent::Create(newParent));
    }

    if (outRemap) {
        *outRemap = std::move(idRemap);
    }
}
} // namespace Serialization
