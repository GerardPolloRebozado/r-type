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
#include "UI/UiComponent.hpp"
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

template <typename T>
void CaptureRegistered(Engine::Registry& registry, Engine::Entity entity, std::optional<T>& slot) {
    if (registry.HasComponent<T>(entity)) {
        slot = registry.RequireComponent<T>(entity);
    }
}

// Plugins that a game may never load. HasComponent throws if the type is missing.
template <typename T>
void CaptureIfRegistered(Engine::Registry& registry, Engine::Entity entity,
                         std::optional<T>& slot) {
    if (!registry.IsComponentRegistered<T>()) {
        return;
    }
    CaptureRegistered(registry, entity, slot);
}

template <typename T>
void Restore(Engine::Registry& registry, Engine::Entity entity, const std::optional<T>& slot) {
    if (slot.has_value()) {
        registry.AddComponent<T>(entity, slot.value());
    }
}

template <typename T>
void RestoreAndRegister(Engine::Registry& registry, Engine::Entity entity,
                        const std::optional<T>& slot) {
    if (!slot.has_value()) {
        return;
    }
    registry.RegisterComponent<T>();
    registry.AddComponent<T>(entity, slot.value());
}

bool SnapshotHasData(const GlazeSerializer::EntitySnapshot& entitySnapshot) {
    return entitySnapshot.name.has_value() || entitySnapshot.position.has_value() ||
           entitySnapshot.rotation.has_value() || entitySnapshot.sprite.has_value() ||
           entitySnapshot.shape.has_value() || entitySnapshot.text.has_value() ||
           entitySnapshot.collider.has_value() || entitySnapshot.opacity.has_value() ||
           entitySnapshot.connections.has_value() || entitySnapshot.parent.has_value() ||
           entitySnapshot.input.has_value() || entitySnapshot.audioSource.has_value() ||
           entitySnapshot.camera.has_value() || entitySnapshot.script.has_value() ||
           entitySnapshot.ui.has_value() || entitySnapshot.prefab.has_value();
}
} // namespace

GlazeSerializer::EntitySnapshot GlazeSerializer::CaptureEntity(Engine::Registry& registry,
                                                               Engine::Entity entity) {
    EntitySnapshot entitySnapshot;

    entitySnapshot.entityId = entity;
    CaptureRegistered(registry, entity, entitySnapshot.name);
    CaptureRegistered(registry, entity, entitySnapshot.position);
    CaptureRegistered(registry, entity, entitySnapshot.rotation);
    CaptureRegistered(registry, entity, entitySnapshot.sprite);
    CaptureRegistered(registry, entity, entitySnapshot.shape);
    CaptureRegistered(registry, entity, entitySnapshot.text);
    CaptureRegistered(registry, entity, entitySnapshot.collider);
    CaptureRegistered(registry, entity, entitySnapshot.opacity);
    CaptureRegistered(registry, entity, entitySnapshot.connections);
    CaptureRegistered(registry, entity, entitySnapshot.parent);
    CaptureRegistered(registry, entity, entitySnapshot.input);
    CaptureRegistered(registry, entity, entitySnapshot.audioSource);
    CaptureRegistered(registry, entity, entitySnapshot.camera);
    CaptureRegistered(registry, entity, entitySnapshot.script);
    // Games that never load the UI plugin have no such component type.
    CaptureIfRegistered(registry, entity, entitySnapshot.ui);
    CaptureIfRegistered(registry, entity, entitySnapshot.network);
    CaptureIfRegistered(registry, entity, entitySnapshot.animation);
    // Tests and tools that never register this type must still capture a scene.
    CaptureIfRegistered(registry, entity, entitySnapshot.prefab);

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

        Restore(registry, entity, entitySnapshot.name);
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
        Restore(registry, entity, entitySnapshot.rotation);
        Restore(registry, entity, entitySnapshot.sprite);
        Restore(registry, entity, entitySnapshot.shape);
        Restore(registry, entity, entitySnapshot.text);
        Restore(registry, entity, entitySnapshot.collider);
        Restore(registry, entity, entitySnapshot.opacity);
        Restore(registry, entity, entitySnapshot.connections);
        Restore(registry, entity, entitySnapshot.input);
        Restore(registry, entity, entitySnapshot.audioSource);
        Restore(registry, entity, entitySnapshot.camera);
        Restore(registry, entity, entitySnapshot.script);
        if (entitySnapshot.ui.has_value()) {
            Engine::UiComponent ui = entitySnapshot.ui.value();
            // loaded and the Lua table are runtime only. The next frame loads the script again.
            ui.loaded = false;
            ui.scriptTable = sol::lua_nil;
            registry.RegisterComponent<Engine::UiComponent>();
            registry.AddComponent<Engine::UiComponent>(entity, ui);
        }
        Restore(registry, entity, entitySnapshot.network);
        RestoreAndRegister(registry, entity, entitySnapshot.prefab);
        RestoreAndRegister(registry, entity, entitySnapshot.animation);
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
                registry.RequireComponent<Engine::SignalConnections>(entity).connections;
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
                registry.RequireComponent<Engine::CameraComponent>(entity).followTarget =
                    it->second;
            }
        }
    }

    return idRemap[entities.front().entityId];
}

void GlazeSerializer::RestoreScene(Engine::Registry& registry, const SceneSnapshot& snapshot) {
    SpawnEntities(registry, snapshot.entities);
}
} // namespace Serialization
