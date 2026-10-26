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
    scene["load"] = [&registry](const std::string& name) {
        if (!registry.HasResource<SceneManager>()) {
            log_error("[Lua] Scene.load('" + name +
                      "'): this game has no SceneManager. Add one in main.cpp "
                      "(Serialization::EnableSceneFiles or AddResource<Engine::SceneManager>).");
            return;
        }
        // In multiplayer the server decides the scene and clients follow it
        // (ReplicationSystem). Scripts often run on every machine, so a client
        // request is ignored instead of making that client leave the session.
        if (registry.HasResource<Multiplayer::NetworkHostResource>() &&
            !registry.GetResource<Multiplayer::NetworkHostResource>().IsServer()) {
            log_debug("[Lua] Scene.load('" + name + "') ignored on a client: the server " +
                      "switches scenes for everyone.");
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
