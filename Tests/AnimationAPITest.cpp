// Lua Animation bindings: add a clip, pick it, pause it, read state, flip it.
#include "Scripting/LuaAPI/Plugins/AnimationAPI.hpp"
#include "Components/Animation.hpp"
#include "ECS/Registry.hpp"
#include "Scripting/LuaAPI/ComponentQueryAPI.hpp"
#include "Scripting/LuaAPI/EntityAPI.hpp"
#include <iostream>
#include <sol/sol.hpp>
#include <string>

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

int main() {
    Engine::Registry registry;
    registry.RegisterComponent<Engine::Animation>();

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);
    Engine::LuaAPI::registerComponentQuery(lua, registry);
    Engine::LuaAPI::registerEntity(lua, registry);
    Engine::LuaAPI::registerAnimation(lua, registry);

    const Engine::Entity entity = registry.CreateEntity();
    lua["e"] = entity;

    // Runs a statement; true when Lua reports no error.
    const auto ok = [&lua](const std::string& code) -> bool {
        return lua.script(code, &sol::script_pass_on_error).valid();
    };
    // Evaluates an expression and returns its boolean result.
    const auto eval = [&lua](const std::string& expression) -> bool {
        auto result = lua.script("return " + expression, &sol::script_pass_on_error);
        return result.valid() && result.get<bool>();
    };

    // A multi-file clip does not touch the filesystem: frames load lazily.
    CHECK(ok("Component.addAnimation(e, {'a.png', 'b.png'}, nil, nil, nil, 0.1, true)"));
    CHECK(registry.HasComponent<Engine::Animation>(entity));
    CHECK(eval("Component.has(e, 'Animation')"));

    CHECK(eval("Component.playAnimation(e, 'Idle')"));
    CHECK(eval("Component.getAnimation(e) == 'Idle'"));
    CHECK(eval("Component.isAnimationPlaying(e)"));
    CHECK(eval("Component.getAnimationFrame(e) == 0"));
    CHECK(eval("Component.getAnimationClips(e)[1] == 'Idle'"));
    CHECK(eval("Component.setAnimationPlaying(e, false)"));
    CHECK(eval("Component.isAnimationPlaying(e) == false"));
    CHECK(!eval("Component.playAnimation(e, 'Missing')"));

    CHECK(eval("Component.setAnimationFlip(e, true, true)"));
    CHECK(eval("(function() local f = Component.getAnimationFlip(e); "
               "return f.flipX == true and f.flipY == true end)()"));

    // A spritesheet needs positive frame dimensions; refuse it otherwise.
    CHECK(!eval("Component.addAnimation(e, 'sheet.png')"));
    CHECK(eval("Component.addAnimation(e, 'sheet.png', 32, 32, 4, 0.05, false)"));

    // Getters answer nil instead of erroring on entities without animation.
    CHECK(eval("(function() local other = Entity.create()\n"
               "return Component.getAnimation(other) == nil\n"
               "  and Component.getAnimationClips(other) == nil\n"
               "  and Component.getAnimationFlip(other) == nil\n"
               "  and Component.getAnimationFrame(other) == nil\n"
               "  and Component.isAnimationPlaying(other) == false end)()"));

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "AnimationAPITest: all checks passed\n";
    return 0;
}
