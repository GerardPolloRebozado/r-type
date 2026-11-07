#include "Scripting/LuaAPI/Plugins/InputAPI.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"
#include <raylib.h>
#include <sol/sol.hpp>

namespace Engine::LuaAPI {

#define ADD_KEY(name, val) keys[#name] = val

void registerInput(sol::state_view lua) {
    sol::table input = Internal::ensureNamespace(lua, "Input");

    input["isKeyDown"] = [](int key) -> bool { return IsKeyDown(key); };
    input["isKeyPressed"] = [](int key) -> bool { return IsKeyPressed(key); };
    input["isKeyReleased"] = [](int key) -> bool { return IsKeyReleased(key); };

    input["getMousePosition"] = [luaState = lua.lua_state()]() mutable -> sol::table {
        sol::state_view lua(luaState);
        sol::table t = lua.create_table();
        t["x"] = static_cast<float>(GetMouseX());
        t["y"] = static_cast<float>(GetMouseY());
        return t;
    };

    sol::table keys = Internal::ensureNamespace(lua, "KEY");
    ADD_KEY(SPACE, 32);
    ADD_KEY(APOSTROPHE, 39);
    ADD_KEY(COMMA, 44);
    ADD_KEY(MINUS, 45);
    ADD_KEY(PERIOD, 46);
    ADD_KEY(SLASH, 47);
    ADD_KEY(ZERO, 48);
    ADD_KEY(ONE, 49);
    ADD_KEY(TWO, 50);
    ADD_KEY(THREE, 51);
    ADD_KEY(FOUR, 52);
    ADD_KEY(FIVE, 53);
    ADD_KEY(SIX, 54);
    ADD_KEY(SEVEN, 55);
    ADD_KEY(EIGHT, 56);
    ADD_KEY(NINE, 57);
    ADD_KEY(SEMICOLON, 59);
    ADD_KEY(EQUAL, 61);
    ADD_KEY(A, 65);
    ADD_KEY(B, 66);
    ADD_KEY(C, 67);
    ADD_KEY(D, 68);
    ADD_KEY(E, 69);
    ADD_KEY(F, 70);
    ADD_KEY(G, 71);
    ADD_KEY(H, 72);
    ADD_KEY(I, 73);
    ADD_KEY(J, 74);
    ADD_KEY(K, 75);
    ADD_KEY(L, 76);
    ADD_KEY(M, 77);
    ADD_KEY(N, 78);
    ADD_KEY(O, 79);
    ADD_KEY(P, 80);
    ADD_KEY(Q, 81);
    ADD_KEY(R, 82);
    ADD_KEY(S, 83);
    ADD_KEY(T, 84);
    ADD_KEY(U, 85);
    ADD_KEY(V, 86);
    ADD_KEY(W, 87);
    ADD_KEY(X, 88);
    ADD_KEY(Y, 89);
    ADD_KEY(Z, 90);
    ADD_KEY(LEFT_BRACKET, 91);
    ADD_KEY(BACKSLASH, 92);
    ADD_KEY(RIGHT_BRACKET, 93);
    ADD_KEY(GRAVE_ACCENT, 96);
    ADD_KEY(ESCAPE, 256);
    ADD_KEY(ENTER, 257);
    ADD_KEY(TAB, 258);
    ADD_KEY(BACKSPACE, 259);
    ADD_KEY(INSERT, 260);
    ADD_KEY(DELETE, 261);
    ADD_KEY(RIGHT, 262);
    ADD_KEY(LEFT, 263);
    ADD_KEY(DOWN, 264);
    ADD_KEY(UP, 265);
    ADD_KEY(PAGE_UP, 266);
    ADD_KEY(PAGE_DOWN, 267);
    ADD_KEY(HOME, 268);
    ADD_KEY(END, 269);
    ADD_KEY(CAPS_LOCK, 280);
    ADD_KEY(SCROLL_LOCK, 281);
    ADD_KEY(NUM_LOCK, 282);
    ADD_KEY(PRINT_SCREEN, 283);
    ADD_KEY(PAUSE, 284);
    ADD_KEY(F1, 290);
    ADD_KEY(F2, 291);
    ADD_KEY(F3, 292);
    ADD_KEY(F4, 293);
    ADD_KEY(F5, 294);
    ADD_KEY(F6, 295);
    ADD_KEY(F7, 296);
    ADD_KEY(F8, 297);
    ADD_KEY(F9, 298);
    ADD_KEY(F10, 299);
    ADD_KEY(F11, 300);
    ADD_KEY(F12, 301);
    ADD_KEY(LEFT_SHIFT, 340);
    ADD_KEY(LEFT_CONTROL, 341);
    ADD_KEY(LEFT_ALT, 342);
    ADD_KEY(RIGHT_SHIFT, 344);
    ADD_KEY(RIGHT_CONTROL, 345);
    ADD_KEY(RIGHT_ALT, 346);
#undef ADD_KEY
}

} // namespace Engine::LuaAPI
