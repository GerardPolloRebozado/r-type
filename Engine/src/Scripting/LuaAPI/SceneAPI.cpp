#include "Scripting/LuaAPI/SceneAPI.hpp"
#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/SceneManager.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {

void registerScene(sol::state_view lua, Registry& registry) {
    sol::table scene = Internal::ensureNamespace(lua, "Scene");

    // Scene.load("Level2.scene") or Scene.load("menu"): switches at the end
    // of the frame, so the rest of the current frame still runs normally.
    // Any connected peer (server or client) may request a scene change;
    // the server initiates level transitions and clients follow via the
    // start_game broadcast, so all peers load the same scene together.
    scene["load"] = [&registry](const std::string& name) {
        if (!registry.HasResource<SceneManager>()) {
            log_error("[Lua] Scene.load('" + name +
                      "'): this game has no SceneManager. Add one in main.cpp "
                      "(Serialization::EnableSceneFiles or AddResource<Engine::SceneManager>).");
            return;
        }
        registry.GetResource<SceneManager>().Load(name);
    };

    // Scene.current(): name of the loaded scene, or "" if none.
    scene["current"] = [&registry]() -> std::string {
        if (!registry.HasResource<SceneManager>()) {
            return "";
        }
        return registry.GetResource<SceneManager>().Current();
    };
}

} // namespace Engine::LuaAPI
