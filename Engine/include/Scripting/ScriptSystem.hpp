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
    /**
     * @brief Calls onDestroy on every loaded script before the scene's entities go away.
     *
     * The Lua state itself is kept: globals set by scripts survive a scene
     * switch, while each script's own table is dropped with its entity.
     */
    void OnSceneUnloaded(Registry& registry) override;
    void loadScript(Registry& registry, Entity entity);
    void unloadScript(Registry& registry, Entity entity);

  private:
    sol::state lua; // Single Lua state shared by all scripts
    bool apiRegistered = false;

    // Track which missing scripts we've already warned about
    static std::unordered_set<std::string> s_warnedMissingScripts;
};
} // namespace Engine

#endif // ENGINE_SCRIPTING_SCRIPTSYSTEM_HPP_
