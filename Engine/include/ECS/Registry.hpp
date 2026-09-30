#ifndef ENGINE_ECS_REGISTRY_HPP_
#define ENGINE_ECS_REGISTRY_HPP_
#include <iostream>

#include "Entity.hpp"
#include "System.hpp"
#include <memory>
#include <stdexcept>
#include <vector>
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
    template<typename T>
    class ComponentArray : public IComponentArray {
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

        bool HasData(Entity entity) const {
            return m_ComponentMap.find(entity) != m_ComponentMap.end();
        }

        /**
         * @brief Retrieves the component for a given entity.
         * @param entity The entity ID.
         * @return Reference to the component data.
         */
        T &GetData(Entity entity) {
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
            return {m_NextEntityId++};
        }


        /**
         * @brief Destroys an entity, removing it from all systems and deleting its components.
         * @param entity The ID of the entity to destroy.
         */
        void DestroyEntity(const Entity entity) {
            for (auto const &pair: m_ComponentArrays) {
                pair.second->EntityDestroyed(entity);
            }
        }

        /**
         * @brief Registers a component type with the registry.
         * @tparam T The component type.
         */
        template<typename T>
        void RegisterComponent() {
            std::type_index typeName = typeid(T);
            if (!m_ComponentArrays.contains(typeName)) {
                m_ComponentArrays[typeName] = std::make_shared<ComponentArray<T> >();
            }
        }

        /**
         * @brief Adds a component to an entity.
         * @tparam T The component type.
         * @param entity The entity ID.
         * @param component The component data.
         */
        template<typename T>
        void AddComponent(Entity entity, T component) {
            GetComponentArray<T>()->InsertData(entity, component);
        }

        /**
         * @brief Removes a component from an entity.
         * @tparam T The component type.
         * @param entity The entity ID.
         */
        template<typename T>
        void RemoveComponent(Entity entity) {
            GetComponentArray<T>()->RemoveData(entity);
        }

        /**
         * @brief Retrieves a component for a specific entity.
         * @tparam T The component type.
         * @param entity The entity ID.
         * @return Reference to the requested component.
         */
        template<typename T>
        T &GetComponent(Entity entity) {
            return GetComponentArray<T>()->GetData(entity);
        }

        /**
         * @brief Checks if an entity has a specific component.
         */
        template<typename T>
        bool HasComponent(Entity entity) {
            return GetComponentArray<T>()->HasData(entity);
        }

        /**
         * @brief Defers entity destruction to the end of the frame.
         * Safe to call inside physics/collision callbacks!
         */
        void QueueDestroyEntity(Entity entity) {
            m_EntitiesToDestroy.push_back(entity);
        }

        /**
         * @brief Safely executes all deferred destructions. Call at the end of the frame.
         */
        void FlushQueues() {
            for (Entity e: m_EntitiesToDestroy) {
                DestroyEntity(e);
            }
            m_EntitiesToDestroy.clear();
        }

        /**
         * @brief Registers a system type with the registry.
         * @tparam T The system type.
         * @return Shared pointer to the registered system.
         */
        template<typename T>
        std::shared_ptr<T> RegisterSystem() {
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
        template<typename T>
        std::shared_ptr<T> GetSystem() {
            std::type_index typeName = typeid(T);
            return std::static_pointer_cast<T>(m_Systems.at(typeName));
        }

        /**
        * @brief Helper function to get the strongly-typed component array.
        * @tparam T The component type.
        * @return Shared pointer to the ComponentArray.
        */
        template<typename T>
        std::shared_ptr<ComponentArray<T> > GetComponentArray() {
            const std::type_index typeName = typeid(T);
            if (!m_ComponentArrays.contains(typeName)) {
                throw std::runtime_error("Component not registered before use.");
            }
            return std::static_pointer_cast<ComponentArray<T> >(m_ComponentArrays[typeName]);
        }

        /**
         * @brief Returns all registered systems.
         */
        const std::unordered_map<std::type_index, std::shared_ptr<System> > &GetSystems() const {
            return m_Systems;
        }

        /**
         * @brief Returns the total number of entities created (used by the Editor for iterating).
         */
        size_t GetEntityCount() const {
            return m_NextEntityId;
        }

        /**
         * @brief Registers a global resource.
         * @tparam T The resource type.
         * @tparam Args Constructor argument types.
         * @return Reference to the created resource.
         */
        template <typename T, typename... Args>
        T& EmplaceResource(Args&&... args) {
            auto res = std::make_shared<T>(std::forward<Args>(args)...);
            m_Resources[typeid(T)] = res;
            return *res;
        }

        /**
         * @brief Retrieves a global resource.
         * @tparam T The resource type.
         * @return Reference to the resource.
         */
        template <typename T>
        T& GetResource() {
            if (!HasResource<T>()) {
                throw std::runtime_error("Resource not found in registry.");
            }
            return *std::static_pointer_cast<T>(m_Resources.at(typeid(T)));
        }
        
        /**
         * @brief Checks if a resource exists.
         * @tparam T The resource type.
         * @return True if the resource exists.
         */
        template <typename T>
        bool HasResource() const {
            return m_Resources.contains(typeid(T));
        }

    private:
        Entity m_NextEntityId = 0;
        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray> > m_ComponentArrays;
        std::unordered_map<std::type_index, std::shared_ptr<System> > m_Systems;
        std::unordered_map<std::type_index, std::shared_ptr<void>> m_Resources;
        std::vector<Entity> m_EntitiesToDestroy;
    };
} // namespace Engine

#endif // ENGINE_ECS_REGISTRY_HPP_
