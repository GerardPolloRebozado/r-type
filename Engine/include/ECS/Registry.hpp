#ifndef ENGINE_ECS_REGISTRY_HPP_
#define ENGINE_ECS_REGISTRY_HPP_
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "Entity.hpp"
#include "Errors/EcsErrors.hpp"
#include "System.hpp"
#include <algorithm>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Resources/StateResource.hpp"

namespace Engine {
/**
 * @class IComponentArray
 * @brief Interface for component arrays so they can be stored generically.
 */
class IComponentArray {
  public:
    virtual ~IComponentArray() = default;

    /**
     * @brief Returns whether an entity has a component in this array.
     */
    virtual bool Has(Entity entity) const = 0;

    /**
     * @brief Removes the component of an entity from this array.
     */
    virtual void Remove(Entity entity) = 0;

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
        m_ComponentMap.insert_or_assign(entity, std::move(component));
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

    bool Has(Entity entity) const override {
        return HasData(entity);
    }

    void Remove(Entity entity) override {
        RemoveData(entity);
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

    auto begin() {
        return m_ComponentMap.begin();
    }

    auto end() {
        return m_ComponentMap.end();
    }

    auto begin() const {
        return m_ComponentMap.begin();
    }

    auto end() const {
        return m_ComponentMap.end();
    }

    void clear() {
        m_ComponentMap.clear();
    }

    size_t size() const {
        return m_ComponentMap.size();
    }

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
    Registry() {
        RegisterComponent<Position>();
        RegisterComponent<Rotation>();
        RegisterComponent<ScreenSpace>();
    }

    /**
     * @brief Creates a new entity and gives it a default Position and Rotation.
     * @return The ID of the newly created entity.
     */
    Entity CreateEntity() {
        Entity entity = {m_NextEntityId++};
        AddComponent<Position>(entity, Position::Create(0, 0));
        AddComponent<Rotation>(entity, Rotation::Create(0.0f));
        return entity;
    }

    /**
     * @brief Destroys an entity and its entire subtree (entities with @ref Parent pointing here).
     * @param entity The ID of the entity to destroy.
     */
    void DestroyEntity(const Entity entity) {
        DestroyEntityWithDescendants(entity);
    }

    /**
     * @brief Registers a component type with the registry.
     * @tparam T The component type.
     */
    template <typename T> Registry* RegisterComponent() {
        const std::type_index typeName = typeid(T);
        if (!m_ComponentArrays.contains(typeName)) {
            m_ComponentArrays[typeName] = std::make_shared<ComponentArray<T>>();
        }
        return this;
    }

    /**
     * @brief Checks if a component type has been registered.
     * @tparam T The component type.
     * @return True if RegisterComponent<T>() has been called.
     */
    template <typename T> bool IsComponentRegistered() const {
        return m_ComponentArrays.contains(typeid(T));
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
     * @brief Checks if an entity has a specific component.
     */
    template <typename T> bool HasComponent(Entity entity) {
        return GetComponentArray<T>()->HasData(entity);
    }

    /**
     * @brief Type-erased component existence check used by scripting glue.
     * @param type The component type index.
     * @param entity The entity ID.
     * @return True if the component is registered and attached to the entity.
     */
    bool HasComponentByType(const std::type_index& type, Entity entity) const {
        const auto it = m_ComponentArrays.find(type);
        if (it == m_ComponentArrays.end()) {
            return false;
        }
        return it->second->Has(entity);
    }

    /**
     * @brief Checks if an entity is alive (holds at least one component).
     *
     * Entity IDs are never reused, so destroyed entities keep their ID but
     * own no components. Editor panels use this to skip dead rows.
     * @param entity The entity ID.
     * @return True if any component is attached to the entity.
     */
    bool IsEntityAlive(Entity entity) const {
        for (const auto& pair : m_ComponentArrays) {
            if (pair.second->Has(entity)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Type-erased component removal used by scripting glue.
     * @param type The component type index.
     * @param entity The entity ID.
     */
    void RemoveComponentByType(const std::type_index& type, Entity entity) {
        const auto it = m_ComponentArrays.find(type);
        if (it == m_ComponentArrays.end()) {
            return;
        }
        it->second->Remove(entity);
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
        for (Entity e : m_EntitiesToDestroy) {
            DestroyEntity(e);
        }
        m_EntitiesToDestroy.clear();
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
        if (std::find(m_SystemUpdateOrder.begin(), m_SystemUpdateOrder.end(), typeName) ==
            m_SystemUpdateOrder.end()) {
            m_SystemUpdateOrder.push_back(typeName);
            m_SystemRenderOrder.push_back(typeName);
        }
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
     * @brief Helper function to get the strongly-typed component array.
     * @tparam T The component type.
     * @return Shared pointer to the ComponentArray.
     */
    template <typename T> std::shared_ptr<ComponentArray<T>> GetComponentArray() {
        const std::type_index typeName = typeid(T);
        if (!m_ComponentArrays.contains(typeName)) {
            throw ErrorEcs("GetComponentArray: component not registered before use: " +
                           std::string(typeName.name()));
        }
        return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
    }

    /**
     * @brief Returns all registered systems.
     */
    const std::unordered_map<std::type_index, std::shared_ptr<System>>& GetSystems() const {
        return m_Systems;
    }

    /**
     * @brief Registration order for @ref System::Update (stable across frames).
     */
    const std::vector<std::type_index>& GetSystemUpdateOrder() const {
        return m_SystemUpdateOrder;
    }

    /**
     * @brief Registration order for @ref System::Render (stable across frames).
     */
    const std::vector<std::type_index>& GetSystemRenderOrder() const {
        return m_SystemRenderOrder;
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
    template <typename T, typename... Args> T& AddResource(Args&&... args) {
        auto res = std::make_shared<T>(std::forward<Args>(args)...);
        m_Resources[typeid(T)] = res;
        return *res;
    }

    /**
     * @brief Retrieves a global resource.
     * @tparam T The resource type.
     * @return Reference to the resource.
     */
    template <typename T> T& GetResource() {
        if (!HasResource<T>()) {
            throw ErrorEcs("GetResource: resource not found in registry.");
        }
        return *std::static_pointer_cast<T>(m_Resources.at(typeid(T)));
    }

    /**
     * @brief Checks if a resource exists.
     * @tparam T The resource type.
     * @return True if the resource exists.
     */
    template <typename T> bool HasResource() const {
        return m_Resources.contains(typeid(T));
    }

  private:
    /**
     * @brief Recursively destroys descendants first (Parent scan), then strips @p entity
     * components.
     *
     * Same behavior for immediate @ref DestroyEntity and deferred @ref QueueDestroyEntity /
     * @ref FlushQueues so grouped hierarchies never leave orphaned children.
     */
    void DestroyEntityWithDescendants(Entity entity) {
        if (m_ComponentArrays.contains(typeid(Parent))) {
            auto parentArray = GetComponentArray<Parent>();
            std::vector<Entity> children;

            children.reserve(parentArray->size());

            for (auto const& entry : *parentArray) {
                if (entry.second.parent == entity) {
                    children.push_back(entry.first);
                }
            }

            for (Entity const child : children) {
                DestroyEntityWithDescendants(child);
            }
        }

        for (auto const& pair : m_ComponentArrays) {
            pair.second->EntityDestroyed(entity);
        }
    }

    Entity m_NextEntityId = 1;
    std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> m_ComponentArrays;
    std::unordered_map<std::type_index, std::shared_ptr<System>> m_Systems;
    std::vector<std::type_index> m_SystemUpdateOrder;
    std::vector<std::type_index> m_SystemRenderOrder;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_Resources;
    std::vector<Entity> m_EntitiesToDestroy;
};
} // namespace Engine

#endif // ENGINE_ECS_REGISTRY_HPP_
