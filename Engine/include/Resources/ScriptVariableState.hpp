#ifndef ENGINE_RESOURCES_SCRIPTVARIABLESTATE_HPP_
#define ENGINE_RESOURCES_SCRIPTVARIABLESTATE_HPP_

#include "ECS/Entity.hpp"

#include <cstddef>
#include <glaze/glaze.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Engine::Multiplayer {

/**
 * @struct ScriptVariableState
 * @brief Per-entity fields replicated automatically by ReplicationSystem.
 */
struct ScriptVariableState {
    struct Registration {
        std::vector<std::string> fields;
    };

    std::unordered_map<Entity, Registration> byEntity;

    /** Values received before the script registered, keyed by network id. */
    std::unordered_map<std::size_t, glz::generic::object_t> pendingByNetworkId;

    Registration& Register(const Entity entity, std::vector<std::string> fields) {
        auto& registration = byEntity[entity];
        registration.fields = std::move(fields);
        return registration;
    }

    Registration* Find(Entity entity) {
        const auto it = byEntity.find(entity);
        return it == byEntity.end() ? nullptr : &it->second;
    }

    const Registration* Find(Entity entity) const {
        const auto it = byEntity.find(entity);
        return it == byEntity.end() ? nullptr : &it->second;
    }

    void Remove(Entity entity) {
        byEntity.erase(entity);
    }

    void RemovePending(std::size_t networkId) {
        pendingByNetworkId.erase(networkId);
    }

    void Reset() {
        byEntity.clear();
        pendingByNetworkId.clear();
    }
};

} // namespace Engine::Multiplayer

#endif // ENGINE_RESOURCES_SCRIPTVARIABLESTATE_HPP_