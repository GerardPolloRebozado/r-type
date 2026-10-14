#ifndef ENGINE_SCRIPTING_SCRIPTSYSTEM_HPP_
#define ENGINE_SCRIPTING_SCRIPTSYSTEM_HPP_

#include "ECS/System.hpp"
#include <sol/sol.hpp>
#include <unordered_set>

namespace Engine {
class Registry;

/**
 * @class ScriptSystem
 * @brief Updates all entities with a ScriptComponent by calling their Lua onUpdate(dt) function.
 *
 * Also handles script loading (onCreate) and unloading (onDestroy).
 */
class ScriptSystem : public System {
  public:
    ScriptSystem();
    ~ScriptSystem();

    void Update(Registry& registry, float dt) override;
    void Render(Registry& registry) override {} // No rendering

  private:
    sol::state lua; // Single Lua state shared by all scripts
    bool apiRegistered = false;

    void loadScript(Registry& registry, Entity entity);
    void unloadScript(Registry& registry, Entity entity);

    // Track which missing scripts we've already warned about
    static std::unordered_set<std::string> s_warnedMissingScripts;
};
} // namespace Engine

#endif // ENGINE_SCRIPTING_SCRIPTSYSTEM_HPP_
