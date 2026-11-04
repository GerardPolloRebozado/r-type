#include "Scripting/LuaAPI/Plugins/ControllerAPI.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/Controller.hpp"
#include "Inputs/IController.hpp"
#include "Inputs/InputManager.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <memory>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {
namespace {
constexpr int kDefaultPlayerId = 1;

IController* getController(Registry& registry, int playerId) {
    if (!registry.HasResource<InputManager>()) {
        return nullptr;
    }
    return registry.GetResource<InputManager>().GetController(playerId);
}

IController* getOrCreateController(Registry& registry, int playerId) {
    if (!registry.HasResource<InputManager>()) {
        registry.AddResource<InputManager>();
    }
    auto& manager = registry.GetResource<InputManager>();
    IController* ctrl = manager.GetController(playerId);
    if (ctrl == nullptr) {
        manager.AddPlayer(playerId, std::make_unique<Controller>());
        ctrl = manager.GetController(playerId);
    }
    return ctrl;
}
} // namespace

void registerController(sol::state_view lua, Registry& registry) {
    sol::table controller = Internal::ensureNamespace(lua, "Controller");

    // (KEYBOARD=0, MOUSE=1, GAMEPAD=2, TOUCH=3).
    controller["KEYBOARD"] = static_cast<int>(DeviceType::KEYBOARD);
    controller["MOUSE"] = static_cast<int>(DeviceType::MOUSE);
    controller["GAMEPAD"] = static_cast<int>(DeviceType::GAMEPAD);
    controller["TOUCH"] = static_cast<int>(DeviceType::TOUCH);

    controller["hasController"] = [&registry](sol::optional<int> playerIdOpt) -> bool {
        const int playerId = playerIdOpt.value_or(kDefaultPlayerId);
        return getController(registry, playerId) != nullptr;
    };

    controller["isActionActive"] = [&registry](const std::string& action,
                                               sol::optional<int> playerIdOpt) -> bool {
        const int playerId = playerIdOpt.value_or(kDefaultPlayerId);
        IController* ctrl = getController(registry, playerId);
        if (ctrl == nullptr) {
            return false;
        }
        return ctrl->isActionActive(action);
    };

    controller["getActiveActions"] = [luaState = lua.lua_state(), &registry](
                                         sol::optional<int> playerIdOpt) mutable -> sol::table {
        sol::state_view lua(luaState);
        sol::table out = lua.create_table();
        const int playerId = playerIdOpt.value_or(kDefaultPlayerId);
        IController* ctrl = getController(registry, playerId);
        if (ctrl == nullptr) {
            return out;
        }
        const auto& active = ctrl->getActiveActions();
        int i = 0;
        for (const auto& action : active) {
            out[i + 1] = action;
            i++;
        }
        return out;
    };

    controller["addKeybind"] =
        [&registry](const std::string& action, int key, sol::optional<int> deviceOpt,
                    const sol::optional<int> composeKeyOpt, const sol::optional<int> playerIdOpt,
                    const sol::optional<bool> networkedOpt) -> bool {
        const int device = deviceOpt.value_or(static_cast<int>(KEYBOARD));
        if (device < static_cast<int>(KEYBOARD) || device > static_cast<int>(TOUCH)) {
            return false;
        }
        const int playerId = playerIdOpt.value_or(kDefaultPlayerId);
        IController* ctrl = getOrCreateController(registry, playerId);
        if (ctrl == nullptr) {
            return false;
        }
        Keybind kb;
        kb.key = key;
        kb.composeKey = composeKeyOpt.value_or(0);
        kb.type = static_cast<DeviceType>(device);
        ctrl->addKeybind(action, kb, networkedOpt.value_or(false));
        return true;
    };
}
} // namespace Engine::LuaAPI
