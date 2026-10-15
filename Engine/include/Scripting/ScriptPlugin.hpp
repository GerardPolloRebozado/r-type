#ifndef ENGINE_SCRIPTING_SCRIPTPLUGIN_HPP_
#define ENGINE_SCRIPTING_SCRIPTPLUGIN_HPP_

#include "Plugins/IPlugin.hpp"
#include "Scripting/ScriptComponent.hpp"

namespace Engine {
class Registry;

/**
 * @class ScriptPlugin
 * @brief Registers ScriptComponent and ScriptSystem with the registry.
 */
class ScriptPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    static void AttachToEntity(Registry& registry, Entity entity, const ScriptComponent& script);
};
} // namespace Engine

#endif // ENGINE_SCRIPTING_SCRIPTPLUGIN_HPP_