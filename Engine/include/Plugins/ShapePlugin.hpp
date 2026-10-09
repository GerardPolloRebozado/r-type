#ifndef ENGINE_PLUGINS_SHAPEPLUGIN_HPP_
#define ENGINE_PLUGINS_SHAPEPLUGIN_HPP_

#include "Components/Position.hpp"
#include "Components/ShapeComponent.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/ShapeSystem.hpp"

namespace Engine {

/**
 * @class ShapePlugin
 * @brief Plugin that bundles the ShapeComponent and ShapeSystem.
 */
class ShapePlugin : public IPlugin {
  public:
    /**
     * @brief Initializes the ShapePlugin by registering the ShapeComponent and ShapeSystem.
     * @param registry The ECS registry.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches a ShapeComponent to an entity.
     * @param registry The ECS registry.
     * @param entity The entity to attach the shape to.
     * @param shape The shape component to attach.
     */
    static void AttachToEntity(Registry& registry, Entity entity, const ShapeComponent& shape);
};

} // namespace Engine

#endif // ENGINE_PLUGINS_SHAPEPLUGIN_HPP_
