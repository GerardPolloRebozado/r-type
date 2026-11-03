#ifndef UI_LUAAPI_UIAPI_HPP_
#define UI_LUAAPI_UIAPI_HPP_

namespace sol {
class state_view;
}

namespace Engine {
class Registry;

namespace LuaAPI {

/**
 * @brief Registers template helpers on the shared Lua state and opens the
 *        vendored sol2 ImGui binding.
 */
void registerUI(sol::state_view lua, Registry& registry);

} // namespace LuaAPI
} // namespace Engine

#endif // UI_LUAAPI_UIAPI_HPP_
