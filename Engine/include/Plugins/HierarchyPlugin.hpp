#ifndef ENGINE_PLUGINS_HIERARCHYPLUGIN_HPP_
#define ENGINE_PLUGINS_HIERARCHYPLUGIN_HPP_

#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/HierarchySystem.hpp"

namespace Engine {

/**
 * @class HierarchyPlugin
 * @brief Registers @ref Parent and @ref HierarchySystem for parent-child hierarchies.
 *
 * Initialize this plugin before @ref PositionPlugin so @ref HierarchySystem runs before
 * @ref PositionSystem snapshots prevX/prevY.
 */
class HierarchyPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    /**
     * @brief Links @p child under @p parent; optionally marks the child position as relative.
     * @param registry ECS registry with @ref Parent and @ref Position registered.
     * @param child Entity that receives @ref Parent.
     * @param parent Entity referenced by @ref Parent::parent.
     * @param relative When true, sets @ref Position::relative on @p child (requires @ref Position).
     */
    static void AttachChild(Registry& registry, Entity child, Entity parent, bool relative = true);
};
} // namespace Engine

#endif // ENGINE_PLUGINS_HIERARCHYPLUGIN_HPP_
