#pragma once
#include "Entity.hpp"
#include "System.hpp"
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace Engine {
/**
 * @class IComponentArray
 * @brief Interface for component arrays so they can be stored generically.
 */
class IComponentArray {
  public:
    virtual ~IComponentArray() = default;

    /**
     * @brief Called when an entity is destroyed, allows the array to clean up.
     * @param entity The ID of the destroyed entity.
     */
    virtual void EntityDestroyed(Entity entity) = 0;
};

/**
 * @class ComponentArray
 * @brief Template class that stores a specific type of component for entities.
 * @tparam T The type of component to store.
 */
template <typename T> class ComponentArray : public IComponentArray {
  public:
    /**
     * @brief Inserts a component for a given entity.
     * @param entity The entity ID.
     * @param component The component data.
     */
    void InsertData(Entity entity, T component) {
        m_ComponentMap[entity] = component;
    }

    /**
     * @brief Removes a component for a given entity.
     * @param entity The entity ID.
     */
    void RemoveData(Entity entity) {
        m_ComponentMap.erase(entity);
    }

    /**
     * @brief Retrieves the component for a given entity.
     * @param entity The entity ID.
     * @return Reference to the component data.
     */
    T& GetData(Entity entity) {
        return m_ComponentMap.at(entity);
    }

    /**
     * @brief Automatically cleans up components when an entity is destroyed.
     * @param entity The entity ID.
     */
    void EntityDestroyed(Entity entity) override {
        if (m_ComponentMap.find(entity) != m_ComponentMap.end()) {
            RemoveData(entity);
        }
    }

  private:
    std::unordered_map<Entity, T> m_ComponentMap;
};

/**
 * @class Registry
 * @brief The core of the ECS. Manages entities, components, and systems.
 *
 * The registry serves as the coordinator for the entire ECS architecture,
 * providing methods to create entities, attach components, and register systems.
 */
class Registry {
  public:
    /**
     * @brief Creates a new entity.
     * @return The ID of the newly created entity.
     */
    Entity CreateEntity() {
        return m_NextEntityId++;
    }

    /**
     * @brief Destroys an entity, removing it from all systems and deleting its components.
     * @param entity The ID of the entity to destroy.
     */
    void DestroyEntity(Entity entity) {
        for (auto const& pair : m_ComponentArrays) {
            pair.second->EntityDestroyed(entity);
        }
        for (auto const& pair : m_Systems) {
            pair.second->m_Entities.erase(entity);
        }
    }

    /**
     * @brief Registers a component type with the registry.
     * @tparam T The component type.
     */
    template <typename T> void RegisterComponent() {
        std::type_index typeName = typeid(T);
        if (m_ComponentArrays.find(typeName) == m_ComponentArrays.end()) {
            m_ComponentArrays[typeName] = std::make_shared<ComponentArray<T>>();
        }
    }

    /**
     * @brief Adds a component to an entity.
     * @tparam T The component type.
     * @param entity The entity ID.
     * @param component The component data.
     */
    template <typename T> void AddComponent(Entity entity, T component) {
        GetComponentArray<T>()->InsertData(entity, component);
    }

    /**
     * @brief Removes a component from an entity.
     * @tparam T The component type.
     * @param entity The entity ID.
     */
    template <typename T> void RemoveComponent(Entity entity) {
        GetComponentArray<T>()->RemoveData(entity);
    }

    /**
     * @brief Retrieves a component for a specific entity.
     * @tparam T The component type.
     * @param entity The entity ID.
     * @return Reference to the requested component.
     */
    template <typename T> T& GetComponent(Entity entity) {
        return GetComponentArray<T>()->GetData(entity);
    }

    /**
     * @brief Registers a system type with the registry.
     * @tparam T The system type.
     * @return Shared pointer to the registered system.
     */
    template <typename T> std::shared_ptr<T> RegisterSystem() {
        std::type_index typeName = typeid(T);
        auto system = std::make_shared<T>();
        m_Systems[typeName] = system;
        return system;
    }

    /**
     * @brief Gets a registered system.
     * @tparam T The system type.
     * @return Shared pointer to the system.
     */
    template <typename T> std::shared_ptr<T> GetSystem() {
        std::type_index typeName = typeid(T);
        return std::static_pointer_cast<T>(m_Systems.at(typeName));
    }

    /**
     * @brief Manually adds an entity to a system.
     * @tparam T The system type.
     * @param entity The entity ID.
     */
    template <typename T> void AddEntityToSystem(Entity entity) {
        std::type_index typeName = typeid(T);
        m_Systems.at(typeName)->m_Entities.insert(entity);
    }

    /**
     * @brief Manually removes an entity from a system.
     * @tparam T The system type.
     * @param entity The entity ID.
     */
    template <typename T> void RemoveEntityFromSystem(Entity entity) {
        std::type_index typeName = typeid(T);
        m_Systems.at(typeName)->m_Entities.erase(entity);
    }

  private:
    Entity m_NextEntityId = 0;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays;
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_Systems;

    /**
     * @brief Helper function to get the strongly-typed component array.
     * @tparam T The component type.
     * @return Shared pointer to the ComponentArray.
     */
    template <typename T> std::shared_ptr<ComponentArray<T>> GetComponentArray() {
        std::type_index typeName = typeid(T);
        if (m_ComponentArrays.find(typeName) == m_ComponentArrays.end()) {
            throw std::runtime_error("Component not registered before use.");
        }
        return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
    }
};
} // namespace Engine
