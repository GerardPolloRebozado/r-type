/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerAPI.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERAPI_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERAPI_HPP
#include <sol/state_view.hpp>

namespace sol {
class state_view;
}

namespace Engine {
class Registry;
}

namespace Engine::LuaAPI {
void registerMultiplayer(sol::state_view lua, Registry& registry);
} // namespace Engine::LuaAPI

#endif // G_ING_500_BAR_5_1_RTYPE_1_MULTIPLAYERAPI_HPP