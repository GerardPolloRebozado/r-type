#ifndef ENGINE_PLUGINS_TAGPLUGIN_HPP_
#define ENGINE_PLUGINS_TAGPLUGIN_HPP_

#include "Components/Tags.hpp"
#include "Plugins/IPlugin.hpp"
#include "Systems/TagSystem.hpp"

namespace Engine {
/**
 * @class TagPlugin
 * @brief Registers @ref Tags and @ref TagSystem.
 *
 * @ref Registry::CreateEntity already attaches an empty @ref Tags list, so a
 * game can skip this plugin and still call the static @ref TagSystem methods.
 * Init is what exposes the system through @ref Registry::GetSystem.
 */
class TagPlugin : public IPlugin {
  public:
    /**
     * @brief Registers the tag component and the tag system.
     */
    void Init(Registry& registry) override {
        registry.RegisterComponent<Tags>();
        registry.RegisterSystem<TagSystem>();
    }
};
} // namespace Engine

#endif // ENGINE_PLUGINS_TAGPLUGIN_HPP_
