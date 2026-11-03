#include "Scripting/LuaAPI/Internal/LuaValue.hpp"

#include <cmath>
#include <cstddef>
#include <string>

namespace Engine::LuaAPI::Internal {

glz::generic LuaToGeneric(const sol::object& obj, std::set<const void*>& seen, int depth) {
    glz::generic out{};
    switch (obj.get_type()) {
        case sol::type::lua_nil:
        case sol::type::none:
            break;
        case sol::type::boolean:
            out = obj.as<bool>();
            break;
        case sol::type::number:
            out = obj.as<double>();
            break;
        case sol::type::string:
            out = obj.as<std::string>();
            break;
        case sol::type::table: {
            if (depth > 32) {
                break; // null on excessive nesting
            }
            const sol::table table = obj.as<sol::table>();
            const void* pointer = table.pointer();
            if (!seen.insert(pointer).second) {
                break; // prevent loops
            }
            const std::size_t count = table.size();
            bool isArray = count > 0;
            for (std::size_t i = 1; i <= count && isArray; ++i) {
                const sol::object item = table.get<sol::object>(i);
                if (!item.valid() || item.get_type() == sol::type::lua_nil) {
                    isArray = false;
                }
            }
            if (isArray) {
                glz::generic::array_t items;
                items.reserve(count);
                for (std::size_t i = 1; i <= count; ++i) {
                    items.push_back(LuaToGeneric(table.get<sol::object>(i), seen, depth + 1));
                }
                out = items;
            } else {
                for (const auto& [key, value] : table) {
                    std::string name;
                    if (key.is<std::string>()) {
                        name = key.as<std::string>();
                    } else if (key.is<int>()) {
                        name = std::to_string(key.as<int>());
                    } else if (key.is<double>()) {
                        name = std::to_string(key.as<double>());
                    } else if (key.is<bool>()) {
                        name = key.as<bool>() ? "true" : "false";
                    } else {
                        continue;
                    }
                    out[name] = LuaToGeneric(value, seen, depth + 1);
                }
            }
            seen.erase(pointer);
            break;
        }
        default:
            break; // other things cannot be serialized
    }
    return out;
}

sol::object GenericToLua(sol::state_view lua, const glz::generic& value) {
    if (value.is_null()) {
        return sol::object{};
    }
    if (value.is_boolean()) {
        return sol::make_object(lua, value.get<bool>());
    }
    if (value.is_string()) {
        return sol::make_object(lua, value.get_string());
    }
    if (value.is_number()) {
        const double number = value.get<double>();
        if (std::floor(number) == number && std::abs(number) <= 9007199254740991.0) {
            return sol::make_object(lua, static_cast<long long>(number));
        }
        return sol::make_object(lua, number);
    }
    if (value.is_array()) {
        sol::table table = lua.create_table();
        int index = 1;
        for (const auto& item : value.get_array()) {
            table[index++] = GenericToLua(lua, item);
        }
        return table;
    }
    if (value.is_object()) {
        sol::table table = lua.create_table();
        for (const auto& entry : value.get_object()) {
            table[entry.first] = GenericToLua(lua, entry.second);
        }
        return table;
    }
    return sol::object{};
}

} // namespace Engine::LuaAPI::Internal