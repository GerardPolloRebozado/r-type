#include "GlazeSerializer.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
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

namespace {
bool SnapshotHasData(const GlazeSerializer::EntitySnapshot& entitySnapshot) {
    return entitySnapshot.name.has_value() || entitySnapshot.position.has_value() ||
           entitySnapshot.rotation.has_value() || entitySnapshot.sprite.has_value() ||
           entitySnapshot.shape.has_value() || entitySnapshot.text.has_value() ||
           entitySnapshot.collider.has_value() || entitySnapshot.opacity.has_value() ||
           entitySnapshot.connections.has_value() || entitySnapshot.parent.has_value() ||
           entitySnapshot.input.has_value() || entitySnapshot.audioSource.has_value() ||
           entitySnapshot.camera.has_value() || entitySnapshot.script.has_value() ||
           entitySnapshot.prefab.has_value();
}
} // namespace

GlazeSerializer::EntitySnapshot GlazeSerializer::CaptureEntity(Engine::Registry& registry,
                                                               Engine::Entity entity) {
    EntitySnapshot entitySnapshot;
    entitySnapshot.entityId = entity;

    if (registry.HasComponent<Engine::Name>(entity)) {
        entitySnapshot.name = registry.GetComponent<Engine::Name>(entity);
    }
    if (registry.HasComponent<Engine::Position>(entity)) {
        entitySnapshot.position = registry.GetComponent<Engine::Position>(entity);
    }
    if (registry.HasComponent<Engine::Rotation>(entity)) {
        entitySnapshot.rotation = registry.GetComponent<Engine::Rotation>(entity);
    }
    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        entitySnapshot.sprite = registry.GetComponent<Engine::SpriteComponent>(entity);
    }
    if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
        entitySnapshot.shape = registry.GetComponent<Engine::ShapeComponent>(entity);
    }
    if (registry.HasComponent<Engine::Text>(entity)) {
        entitySnapshot.text = registry.GetComponent<Engine::Text>(entity);
    }
    if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
        entitySnapshot.collider = registry.GetComponent<Engine::ColliderComponent>(entity);
    }
    if (registry.HasComponent<Engine::Opacity>(entity)) {
        entitySnapshot.opacity = registry.GetComponent<Engine::Opacity>(entity);
    }
    if (registry.HasComponent<Engine::SignalConnections>(entity)) {
        entitySnapshot.connections = registry.GetComponent<Engine::SignalConnections>(entity);
    }
    if (registry.HasComponent<Engine::Parent>(entity)) {
        entitySnapshot.parent = registry.GetComponent<Engine::Parent>(entity);
    }
    if (registry.HasComponent<Engine::Input>(entity)) {
        entitySnapshot.input = registry.GetComponent<Engine::Input>(entity);
    }
    if (registry.HasComponent<Engine::AudioSource>(entity)) {
        entitySnapshot.audioSource = registry.GetComponent<Engine::AudioSource>(entity);
    }
    if (registry.HasComponent<Engine::CameraComponent>(entity)) {
        entitySnapshot.camera = registry.GetComponent<Engine::CameraComponent>(entity);
    }
    if (registry.HasComponent<Engine::ScriptComponent>(entity)) {
        entitySnapshot.script = registry.GetComponent<Engine::ScriptComponent>(entity);
    }
    if (registry.IsComponentRegistered<Engine::NetworkComponent>() &&
        registry.HasComponent<Engine::NetworkComponent>(entity)) {
        entitySnapshot.network = registry.GetComponent<Engine::NetworkComponent>(entity);
    }
    // Tests and tools that never register this type must still capture a scene.
    if (registry.IsComponentRegistered<Engine::PrefabInstance>() &&
        registry.HasComponent<Engine::PrefabInstance>(entity)) {
        entitySnapshot.prefab = registry.GetComponent<Engine::PrefabInstance>(entity);
    }

    return entitySnapshot;
}

GlazeSerializer::SceneSnapshot GlazeSerializer::CaptureScene(Engine::Registry& registry) {
    SceneSnapshot snapshot;
    snapshot.sceneName = "Untitled";

    for (size_t i = 0; i < registry.GetEntityCount(); ++i) {
        Engine::Entity entity = i;
        EntitySnapshot entitySnapshot = CaptureEntity(registry, entity);

        // Skip entity slots that were never created or have been destroyed so that
        // restored entity IDs stay compact and parent links can be remapped exactly.
        if (SnapshotHasData(entitySnapshot)) {
            snapshot.entities.push_back(std::move(entitySnapshot));
        }
    }

    return snapshot;
}

Engine::Entity GlazeSerializer::SpawnEntities(Engine::Registry& registry,
                                              const std::vector<EntitySnapshot>& entities) {
    if (entities.empty()) {
        return static_cast<Engine::Entity>(-1);
    }

    // Pass 1: recreate entities and restore every component except Parent, recording
    // the mapping from serialized entity IDs to the freshly created ones.
    std::unordered_map<Engine::Entity, Engine::Entity> idRemap;
    idRemap.reserve(entities.size());

    for (const auto& entitySnapshot : entities) {
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
            restored.prevZ = restored.z;
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
        if (entitySnapshot.connections.has_value()) {
            registry.AddComponent<Engine::SignalConnections>(entity,
                                                             entitySnapshot.connections.value());
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
        if (entitySnapshot.network.has_value()) {
            registry.AddComponent<Engine::NetworkComponent>(entity, entitySnapshot.network.value());
        }
        if (entitySnapshot.prefab.has_value()) {
            registry.RegisterComponent<Engine::PrefabInstance>();
            registry.AddComponent<Engine::PrefabInstance>(entity, entitySnapshot.prefab.value());
        }
    }

    for (const auto& entitySnapshot : entities) {
        const bool remapCamera =
            entitySnapshot.camera.has_value() && entitySnapshot.camera->followTarget.has_value();
        if (!entitySnapshot.parent.has_value() && !entitySnapshot.connections.has_value() &&
            !remapCamera) {
            continue;
        }
        const Engine::Entity entity = idRemap[entitySnapshot.entityId];
        // Ids inside this subtree become the new entities. Ids outside it stay,
        // so a paste keeps the original parent and a signal keeps an external target.
        if (entitySnapshot.parent.has_value()) {
            const Engine::Entity oldParent = entitySnapshot.parent->parent;
            const auto it = idRemap.find(oldParent);
            const Engine::Entity newParent = (it != idRemap.end()) ? it->second : oldParent;
            registry.AddComponent<Engine::Parent>(entity, Engine::Parent::Create(newParent));
        }
        if (entitySnapshot.connections.has_value() &&
            registry.HasComponent<Engine::SignalConnections>(entity)) {
            auto& connections =
                registry.GetComponent<Engine::SignalConnections>(entity).connections;
            for (auto& connection : connections) {
                const auto it = idRemap.find(connection.target);
                if (it != idRemap.end()) {
                    connection.target = it->second;
                }
            }
        }
        if (remapCamera && registry.HasComponent<Engine::CameraComponent>(entity)) {
            const auto it = idRemap.find(*entitySnapshot.camera->followTarget);
            if (it != idRemap.end()) {
                registry.GetComponent<Engine::CameraComponent>(entity).followTarget = it->second;
            }
        }
    }

    return idRemap[entities.front().entityId];
}

void GlazeSerializer::RestoreScene(Engine::Registry& registry, const SceneSnapshot& snapshot) {
    SpawnEntities(registry, snapshot.entities);
}
} // namespace Serialization
