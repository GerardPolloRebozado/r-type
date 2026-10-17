/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** MultiplayerAPI.cpp
*/
#include "Scripting/LuaAPI/Plugins/MultiplayerAPI.hpp"

#include "ECS/Registry.hpp"
#include "Logging/Logger.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRegistryResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glaze/glaze.hpp>
#include <optional>
#include <set>
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {
namespace {

using Engine::Multiplayer::NetworkHostResource;
using Engine::Multiplayer::PacketRegistryResource;
using Engine::Multiplayer::PacketRouterResource;

PacketRegistryResource* GetPacketRegistry(Registry& registry) {
    if (!registry.HasResource<PacketRegistryResource>()) {
        log_error("[Net] PacketRegistryResource missing: did you forget MultiplayerPlugin?");
        return nullptr;
    }
    return &registry.GetResource<PacketRegistryResource>();
}

PacketRouterResource* GetPacketRouter(Registry& registry) {
    if (!registry.HasResource<PacketRouterResource>()) {
        log_error("[Net] PacketRouterResource missing: did you forget MultiplayerPlugin?");
        return nullptr;
    }
    return &registry.GetResource<PacketRouterResource>();
}

// Lua tables are dynamic so reflection is not possible, we use the generic glaze serializers to do
// it
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
                break; // cycle guard -> null
            }
            // Array iff keys are exactly 1..N (Lua convention); otherwise an object
            // with stringified keys (JSON objects only allow string keys).
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
            break; // functions, threads, userdata: not serializable -> null
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

std::string EncodePayload(const sol::object& data) {
    if (!data.valid() || data.get_type() == sol::type::lua_nil ||
        data.get_type() == sol::type::none) {
        return {};
    }
    if (data.is<std::string>()) {
        return data.as<std::string>(); // raw string passthrough (no JSON quoting)
    }
    std::set<const void*> seen;
    const glz::generic generic = LuaToGeneric(data, seen, 0);
    const auto serialized = glz::write_json(generic);
    if (!serialized) {
        log_error("[Net] Failed to encode packet payload to JSON");
        return {};
    }
    return *serialized;
}

sol::object DecodePayload(sol::state_view lua, const uint8_t* payload, std::size_t size) {
    if (size == 0 || payload == nullptr) {
        return sol::object{};
    }
    const std::string raw{reinterpret_cast<const char*>(payload), size};
    glz::generic generic{};
    if (!glz::read_json(generic, raw)) {
        return GenericToLua(lua, generic);
    }
    return sol::make_object(lua, raw); // not JSON: deliver the raw string
}

} // namespace

void registerMultiplayer(sol::state_view lua, Registry& registry) {
    sol::table net = Internal::ensureNamespace(lua, "Net");

    net["register"] = [&registry](sol::this_state ts, const std::string& name) -> sol::object {
        sol::state_view lua(ts);
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<int>(packetRegistry->GetOrRegister(name)));
    };

    net["id"] = [&registry](sol::this_state ts, const std::string& name) -> sol::object {
        // Lookup-only: no registration, so callers can probe safely.
        sol::state_view lua(ts);
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr) {
            return sol::object{};
        }
        const auto found = packetRegistry->TryGet(name);
        if (!found.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<int>(*found));
    };

    net["name"] = [&registry](sol::this_state ts, int id) -> sol::object {
        sol::state_view lua(ts);
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr) {
            return sol::object{};
        }
        const auto found = packetRegistry->GetName(static_cast<PacketTypeID>(id));
        if (!found.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, *found);
    };

    net["startServer"] = [&registry](int port, sol::optional<int> maxClients) -> bool {
        if (registry.HasResource<NetworkHostResource>()) {
            log_error("[Net] startServer: host already started");
            return false;
        }
        if (port <= 0 || port > 65535) {
            log_error("[Net] startServer: invalid port");
            return false;
        }
        const auto clients = static_cast<std::size_t>(std::max(1, maxClients.value_or(32)));
        registry.AddResource<NetworkHostResource>(
            NetworkHostResource::CreateServer(static_cast<std::uint16_t>(port), clients));
        return true;
    };

    net["startClient"] = [&registry](const std::string& host, int port) -> bool {
        if (registry.HasResource<NetworkHostResource>()) {
            log_error("[Net] startClient: host already started");
            return false;
        }
        if (port <= 0 || port > 65535) {
            log_error("[Net] startClient: invalid port");
            return false;
        }
        auto& netHost =
            registry.AddResource<NetworkHostResource>(NetworkHostResource::CreateClient());
        return netHost.Connect(host, static_cast<std::uint16_t>(port)) != nullptr;
    };

    net["on"] = [lua, &registry](const std::string& name, sol::function callback) mutable {
        auto* packetRegistry = GetPacketRegistry(registry);
        auto* router = GetPacketRouter(registry);
        if (packetRegistry == nullptr || router == nullptr || !callback.valid()) {
            return false;
        }
        const PacketTypeID id = packetRegistry->GetOrRegister(name);
        router->On(id, [lua, callback](Registry& /*r*/, ENetPeer* peer, const uint8_t* payload,
                                       const std::size_t size) mutable {
            sol::object data = DecodePayload(lua, payload, size);
            auto result = callback(static_cast<void*>(peer), data);
            if (!result.valid()) {
                sol::error err = result;
                log_error(std::string("[Net] on() handler error: ") + err.what());
            }
        });
        return true;
    };

    net["off"] = [&registry](const std::string& name) {
        auto* packetRegistry = GetPacketRegistry(registry);
        auto* router = GetPacketRouter(registry);
        if (packetRegistry == nullptr || router == nullptr) {
            return false;
        }
        const auto id = packetRegistry->TryGet(name);
        if (!id.has_value()) {
            return false;
        }
        return router->Off(*id);
    };

    net["send"] = [&registry](sol::object peerObj, const std::string& name, sol::object data,
                              sol::optional<bool> reliableOpt, sol::optional<int> channelOpt) {
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr || !registry.HasResource<NetworkHostResource>()) {
            return false;
        }
        void* rawPeer = nullptr;
        if (peerObj.is<void*>()) {
            rawPeer = peerObj.as<void*>();
        } else if (peerObj.is<ENetPeer*>()) {
            rawPeer = peerObj.as<ENetPeer*>();
        }
        auto* peer = static_cast<ENetPeer*>(rawPeer);
        if (peer == nullptr) {
            return false;
        }
        const PacketTypeID id = packetRegistry->GetOrRegister(name);
        const std::string payload = EncodePayload(data);
        const bool reliable = reliableOpt.value_or(false);
        const auto channel = static_cast<uint8_t>(channelOpt.value_or(0));
        ENetPacket* packet = NetworkHostResource::CreateRawPacket(
            id, payload.empty() ? nullptr : reinterpret_cast<const uint8_t*>(payload.data()),
            payload.size(), reliable);
        if (packet == nullptr) {
            return false;
        }
        NetworkHostResource::Send(peer, packet, channel);
        return true;
    };

    net["broadcast"] = [&registry](const std::string& name, sol::object data,
                                   sol::optional<bool> reliableOpt, sol::optional<int> channelOpt) {
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr || !registry.HasResource<NetworkHostResource>()) {
            return false;
        }
        const PacketTypeID id = packetRegistry->GetOrRegister(name);
        const std::string payload = EncodePayload(data);
        const bool reliable = reliableOpt.value_or(false);
        const auto channel = static_cast<uint8_t>(channelOpt.value_or(0));
        ENetPacket* packet = NetworkHostResource::CreateRawPacket(
            id, payload.empty() ? nullptr : reinterpret_cast<const uint8_t*>(payload.data()),
            payload.size(), reliable);
        if (packet == nullptr) {
            return false;
        }
        registry.GetResource<NetworkHostResource>().SendAllHost(packet, channel);
        return true;
    };

    net["onConnect"] = [lua, &registry](sol::object callback) mutable {
        auto* router = GetPacketRouter(registry);
        if (router == nullptr) {
            return;
        }
        if (!callback.valid() || callback.get_type() == sol::type::lua_nil) {
            router->m_OnConnect = nullptr;
            return;
        }
        sol::function fn = callback.as<sol::function>();
        if (!fn.valid()) {
            return;
        }
        router->m_OnConnect = [lua, fn](Registry& /*r*/, ENetPeer* peer) mutable {
            auto result = fn(static_cast<void*>(peer));
            if (!result.valid()) {
                sol::error err = result;
                log_error(std::string("[Net] onConnect error: ") + err.what());
            }
        };
    };

    net["onDisconnect"] = [lua, &registry](sol::object callback) mutable {
        auto* router = GetPacketRouter(registry);
        if (router == nullptr) {
            return;
        }
        if (!callback.valid() || callback.get_type() == sol::type::lua_nil) {
            router->m_OnDisconnect = nullptr;
            return;
        }
        sol::function fn = callback.as<sol::function>();
        if (!fn.valid()) {
            return;
        }
        router->m_OnDisconnect = [lua, fn](Registry& /*r*/, ENetPeer* peer) mutable {
            auto result = fn(static_cast<void*>(peer));
            if (!result.valid()) {
                sol::error err = result;
                log_error(std::string("[Net] onDisconnect error: ") + err.what());
            }
        };
    };

    net["isServer"] = [&registry]() {
        if (!registry.HasResource<NetworkHostResource>()) {
            return false;
        }
        return registry.GetResource<NetworkHostResource>().IsServer();
    };

    net["localId"] = [&registry](sol::this_state ts) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>()) {
            return sol::object{};
        }
        const auto id = registry.GetResource<NetworkHostResource>().GetLocalNetworkId();
        if (!id.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<double>(*id));
    };

    net["peerEntity"] = [](sol::this_state ts, sol::object peerObj) -> sol::object {
        sol::state_view lua(ts);
        void* rawPeer = nullptr;
        if (peerObj.is<void*>()) {
            rawPeer = peerObj.as<void*>();
        } else if (peerObj.is<ENetPeer*>()) {
            rawPeer = peerObj.as<ENetPeer*>();
        }
        const auto entity = NetworkHostResource::GetEntityID(static_cast<ENetPeer*>(rawPeer));
        if (!entity.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<double>(*entity));
    };
}

} // namespace Engine::LuaAPI
