/*
** EPITECH PROJECT, 2026
** G-ING_500_BAR_5_1_RTYPE_1
** File description:
** MultiplayerAPI.cpp
*/
#include "Scripting/LuaAPI/Plugins/MultiplayerAPI.hpp"

#include "Components/NetworkComponent.hpp"
#include "ECS/Registry.hpp"
#include "Inputs/InputManager.hpp"
#include "Logging/Logger.hpp"
#include "Resources/NetworkHostResource.hpp"
#include "Resources/PacketRegistryResource.hpp"
#include "Resources/PacketRouterResource.hpp"
#include "Resources/PlayerInputState.hpp"
#include "Resources/PlayerSlotState.hpp"
#include "Scripting/LuaAPI/Internal/Helpers.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <glaze/glaze.hpp>
#include <optional>
#include <set>
#include <sol/sol.hpp>
#include <string>

namespace Engine::LuaAPI {
using Multiplayer::NetworkHostResource;
using Multiplayer::PacketRegistryResource;
using Multiplayer::PacketRouterResource;
using Multiplayer::PlayerSlotState;

PacketRegistryResource* GetPacketRegistry(Registry& registry) {
    return &registry.GetResource<PacketRegistryResource>();
}

PacketRouterResource* GetPacketRouter(Registry& registry) {
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

void registerMultiplayer(const sol::state_view& lua, Registry& registry) {
    sol::table net = Internal::ensureNamespace(lua, "Net");

    net["register"] = [&registry](const sol::this_state ts,
                                  const std::string& name) -> sol::object {
        sol::state_view lua(ts);
        auto* packetRegistry = GetPacketRegistry(registry);
        if (packetRegistry == nullptr) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<int>(packetRegistry->GetOrRegister(name)));
    };

    net["id"] = [&registry](const sol::this_state ts, const std::string& name) -> sol::object {
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
            log_user_error("[Net] startServer: host already started");
            return false;
        }
        if (port <= 0 || port > 65535) {
            log_user_error("[Net] startServer: invalid port");
            return false;
        }
        const auto clients = static_cast<std::size_t>(std::max(1, maxClients.value_or(32)));
        registry.AddResource<NetworkHostResource>(
            NetworkHostResource::CreateServer(static_cast<std::uint16_t>(port), clients));
        return true;
    };

    net["startClient"] = [&registry](const std::string& host, int port) -> bool {
        if (registry.HasResource<NetworkHostResource>()) {
            log_user_error("[Net] startClient: host already started");
            return false;
        }
        if (port <= 0 || port > 65535) {
            log_user_error("[Net] startClient: invalid port");
            return false;
        }
        auto& netHost =
            registry.AddResource<NetworkHostResource>(NetworkHostResource::CreateClient());
        return netHost.Connect(host, static_cast<std::uint16_t>(port)) != nullptr;
    };

    net["on"] = [lua, &registry](const std::string& name, const sol::function& callback) mutable {
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
                log_user_error(std::string("[Net] on() handler error: ") + err.what());
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
                log_user_error(std::string("[Net] onConnect error: ") + err.what());
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
                log_user_error(std::string("[Net] onDisconnect error: ") + err.what());
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

    // Network ids are assigned and tracked by the engine (server on connect,
    // client via the your_player_id handshake), so scripts never keep their own
    // id <-> entity tables.
    net["networkId"] = [&registry](sol::this_state ts, Entity entity) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>()) {
            return sol::object{};
        }
        const auto id = registry.GetResource<NetworkHostResource>().GetNetworkId(registry, entity);
        if (!id.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<double>(*id));
    };

    // Color slot (0-based) the server assigned to a player (0 = first color)
    net["playerSlot"] = [&registry](sol::this_state ts, Entity entity) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>() ||
            !registry.HasResource<PlayerSlotState>()) {
            return sol::object{};
        }
        const auto networkId =
            registry.GetResource<NetworkHostResource>().GetNetworkId(registry, entity);
        if (!networkId.has_value()) {
            return sol::object{};
        }
        const auto slot = registry.GetResource<PlayerSlotState>().Get(*networkId);
        if (!slot.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, static_cast<int>(*slot));
    };

    // Number of distinct player colors/slots the game supports.
    net["maxPlayers"] = []() { return static_cast<int>(PlayerSlotState::kMaxPlayers); };

    net["entityFromId"] = [&registry](sol::this_state ts, double networkId) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>()) {
            return sol::object{};
        }
        const auto entity = registry.GetResource<NetworkHostResource>().GetEntityByNetworkId(
            static_cast<std::size_t>(networkId));
        if (!entity.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, *entity);
    };

    net["track"] = [&registry](Entity entity, double networkId) -> bool {
        if (!registry.HasResource<NetworkHostResource>() || !registry.IsEntityAlive(entity)) {
            return false;
        }
        registry.GetResource<NetworkHostResource>().TrackNetworkEntity(
            registry, entity, static_cast<std::size_t>(networkId));
        return true;
    };

    net["untrack"] = [&registry](Entity entity) -> bool {
        if (!registry.HasResource<NetworkHostResource>()) {
            return false;
        }
        registry.GetResource<NetworkHostResource>().UntrackNetworkEntity(registry, entity);
        return true;
    };

    net["localEntity"] = [&registry](sol::this_state ts) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>()) {
            return sol::object{};
        }
        const auto entity = registry.GetResource<NetworkHostResource>().GetLocalEntity();
        if (!entity.has_value()) {
            return sol::object{};
        }
        return sol::make_object(lua, *entity);
    };

    // True when this process created a host (Play Multiplayer), false for
    // single-player. Lets one script support both modes
    net["isNetworked"] = [&registry]() { return registry.HasResource<NetworkHostResource>(); };

    // True when the entity belongs to this client (its network id matches the
    // id the server assigned to this process)
    net["isLocal"] = [&registry](Entity entity) -> bool {
        if (!registry.HasResource<NetworkHostResource>()) {
            return false;
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        const auto localId = host.GetLocalNetworkId();
        const auto entityId = host.GetNetworkId(registry, entity);
        return localId.has_value() && entityId.has_value() && *localId == *entityId;
    };

    /**
     * Input for an entity, as a Lua set-table { UP = true, SHOOT = true, ... }
     * keyed by action name (same names as Controller.isActionActive).
     *
     * - Server: the last input received for that player.
     * - Client: the local controller input when the entity is the local player.
     * - Remote entity / single-player: nil (the caller falls back to Controller).
     *
     * This is what lets gameplay scripts move server-authoritative players
     * without ever touching ENet: player.lua simply reads Net.input(entity).
     */
    net["input"] = [&registry](const sol::this_state ts, const Entity entity) -> sol::object {
        sol::state_view lua(ts);
        if (!registry.HasResource<NetworkHostResource>()) {
            return sol::object{};
        }
        auto& host = registry.GetResource<NetworkHostResource>();
        const auto entityId = host.GetNetworkId(registry, entity);
        if (!entityId.has_value()) {
            return sol::object{};
        }

        auto makeTable = [&lua](const std::set<Action>& input) {
            sol::table table = lua.create_table();
            for (const auto& action : input) {
                table[action] = true;
            }
            return sol::make_object(lua, table);
        };

        if (host.IsServer()) {
            std::set<Action> input{};
            if (registry.HasResource<Multiplayer::PlayerInputState>()) {
                const auto& inputs =
                    registry.GetResource<Multiplayer::PlayerInputState>().byNetworkId;
                if (const auto it = inputs.find(*entityId); it != inputs.end()) {
                    input = it->second;
                }
            }
            return makeTable(input);
        }

        const auto localId = host.GetLocalNetworkId();
        if (!localId.has_value() || *localId != *entityId ||
            !registry.HasResource<InputManager>()) {
            return sol::object{};
        }
        auto* controller = registry.GetResource<InputManager>().GetController(1);
        if (controller == nullptr) {
            return sol::object{};
        }
        return makeTable(controller->getNetworkedActiveActions());
    };

    /**
     * Marks a runtime entity (bullet, effect, ...) as replicated.
     * Net.replicate(entity, "Server", true, true)
     *
     * On the server this assigns a network id immediately; ReplicationSystem
     * announces the spawn and syncs Position/Rotation/Sprite from then on. On
     * a client it only tags the entity (clients never broadcast).
     */
    net["replicate"] = [&registry](Entity entity, sol::optional<std::string> authority,
                                   sol::optional<bool> syncTransform,
                                   sol::optional<bool> syncSprite,
                                   sol::optional<std::string> prefabPath) -> bool {
        if (!registry.IsEntityAlive(entity) ||
            !registry.IsComponentRegistered<NetworkComponent>()) {
            return false;
        }
        NetworkComponent network;
        if (registry.HasComponent<NetworkComponent>(entity)) {
            network = registry.RequireComponent<NetworkComponent>(entity);
        }
        network.authority = authority.value_or("Server");
        network.syncTransform = syncTransform.value_or(true);
        network.syncSprite = syncSprite.value_or(false);
        if (prefabPath.has_value()) {
            network.prefabPath = *prefabPath;
        }

        registry.AddComponent<NetworkComponent>(entity, network);
        if (registry.HasResource<NetworkHostResource>()) {
            auto& host = registry.GetResource<NetworkHostResource>();
            if (host.IsServer() && network.networkId == 0) {
                network.networkId = host.GetNextNetworkId();
                registry.AddComponent<NetworkComponent>(entity, network);
                host.TrackNetworkEntity(registry, entity, network.networkId);
            }
        }
        return true;
    };
}
} // namespace Engine::LuaAPI