#include "Author/Commands.hpp"
#include "Logging/LogWriter.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace Author {
namespace {

constexpr const char* kProtocolVersion = "2024-11-05";

const char* kToolsJson = R"JSON([
  {"name":"project_summary","description":"Summarize a game project: startup scene, scene list, prefabs, entity count.","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string","description":"Game project directory. Defaults to the workspace folder."},"scenePath":{"type":"string","description":"Scene file relative to the project, or empty for project.json startupScene."}}}},
  {"name":"scene_outline","description":"Return a tree of entity names, components, scripts, and UI templates. Never returns raw scene JSON.","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string","description":"Game project directory. Defaults to the workspace folder."},"scenePath":{"type":"string","description":"Scene file relative to the project, or empty for project.json startupScene."}}}},
  {"name":"list_ui_templates","description":"List premade UI templates and their editable fields from the engine catalog.","inputSchema":{"type":"object","additionalProperties":true,"properties":{}}},
  {"name":"list_assets","description":"List files under the project's assets/ folder.","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string","description":"Game project directory. Defaults to the workspace folder."}}}},
  {"name":"list_prefabs","description":"List .prefab files under the project's Prefabs/ folder.","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string","description":"Game project directory. Defaults to the workspace folder."}}}},
  {"name":"lua_api","description":"Return one Lua API doc page (topic like entity, component/sprite, or index).","inputSchema":{"type":"object","additionalProperties":true,"properties":{"topic":{"type":"string","description":"Page under Documentation/docs/lua/, without .md."},"engineRoot":{"type":"string"}}}},
  {"name":"add_ui_screen","description":"Place a UI screen template (menu, settings, confirm) in the scene.","inputSchema":{"type":"object","additionalProperties":true,"required":["template"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"template":{"type":"string","description":"Catalog id: menu, settings, or confirm."},"name":{"type":"string"},"startPage":{"type":"boolean"},"props":{"type":"object"}}}},
  {"name":"add_ui_widget","description":"Add a UI widget as a child of a screen. Use list_ui_templates for ids and props.","inputSchema":{"type":"object","additionalProperties":true,"required":["screen","template"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"screen":{"type":"string","description":"Screen page name or entity name."},"template":{"type":"string","description":"Widget catalog id, e.g. button."},"name":{"type":"string"},"props":{"type":"object"},"onClick":{"type":"object","description":"Action: type none|goToPage|back|callMethod|loadScene|quit, plus target."},"onCancel":{"type":"object"}}}},
  {"name":"set_ui_props","description":"Change props or click actions on an existing UI entity.","inputSchema":{"type":"object","additionalProperties":true,"required":["entity"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"entity":{"type":"string","description":"Entity name or id."},"props":{"type":"object"},"onClick":{"type":"object"},"onCancel":{"type":"object"}}}},
  {"name":"place_prefab","description":"Spawn a prefab into the scene at an optional position.","inputSchema":{"type":"object","additionalProperties":true,"required":["prefabPath"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"prefabPath":{"type":"string","description":"Project-relative path, e.g. Prefabs/Player.prefab."},"x":{"type":"number"},"y":{"type":"number"},"name":{"type":"string"}}}},
  {"name":"create_entity","description":"Create an entity with a name and position. Optional components: [{component, fields}].","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"name":{"type":"string"},"x":{"type":"number"},"y":{"type":"number"},"z":{"type":"integer"},"components":{"type":"array"}}}},
  {"name":"set_component","description":"Add or update one component on an entity. Allowed: position, rotation, name, sprite, collider, shape, text, opacity, camera, audio, input, network.","inputSchema":{"type":"object","additionalProperties":true,"required":["entity","component"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"entity":{"type":"string"},"component":{"type":"string"},"fields":{"type":"object"}}}},
  {"name":"get_component","description":"Read one component's saved fields on an entity, including position x, y and z. Use this before moving an entity next to another. Omit component to read every supported component.","inputSchema":{"type":"object","additionalProperties":true,"required":["entity"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"entity":{"type":"string","description":"Entity name or id."},"component":{"type":"string","description":"position, rotation, name, sprite, collider, shape, text, opacity, camera, audio, input, or network."}}}},
  {"name":"attach_script","description":"Write a Lua script skeleton (hook bodies only) and attach it. Does not overwrite an existing file when hooks are set.","inputSchema":{"type":"object","additionalProperties":true,"required":["entity"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"entity":{"type":"string"},"scriptPath":{"type":"string"},"hooks":{"type":"object","description":"Map of hook name to Lua body, e.g. onUpdate, onCollisionEnter."},"autoReload":{"type":"boolean"}}}},
  {"name":"add_wave","description":"Append an enemy wave to the scene.","inputSchema":{"type":"object","additionalProperties":true,"required":["wave"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"wave":{"type":"object","description":"name, triggerX, prefabPath, formation (line|vformation|sine|grid), count, spacing, baseY, spawnOffsetX, attachParent, groupScriptPath."}}}},
  {"name":"set_wave","description":"Change an existing wave. wave.index comes from scene_outline. Only the fields you send are changed.","inputSchema":{"type":"object","additionalProperties":true,"required":["wave"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"wave":{"type":"object","description":"index, plus any of name, triggerX, prefabPath, formation, count, spacing, baseY, spawnOffsetX, attachParent, groupScriptPath."}}}},
  {"name":"remove_wave","description":"Delete one wave from the scene.","inputSchema":{"type":"object","additionalProperties":true,"required":["wave"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"wave":{"type":"object","description":"index from scene_outline, or name."}}}},
  {"name":"create_prefab","description":"Write a new .prefab from a scene entity and its children. Does not overwrite an existing file.","inputSchema":{"type":"object","additionalProperties":true,"required":["entity"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"entity":{"type":"string"},"name":{"type":"string"},"prefabPath":{"type":"string","description":"Project-relative path. Default Prefabs/<name>.prefab."}}}},
  {"name":"set_prefab_component","description":"Change one component inside an existing prefab file, and on scene instances of that prefab.","inputSchema":{"type":"object","additionalProperties":true,"required":["prefabPath","component"],"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"},"prefabPath":{"type":"string"},"entity":{"type":"string","description":"Entity name inside the prefab. Empty means the root."},"component":{"type":"string"},"fields":{"type":"object"}}}},
  {"name":"validate_scene","description":"Load the scene through SceneSerializer and report parse errors.","inputSchema":{"type":"object","additionalProperties":true,"properties":{"projectFolder":{"type":"string"},"scenePath":{"type":"string"}}}},
  {"name":"validate_script","description":"Load a Lua file and check that it parses and returns a table.","inputSchema":{"type":"object","additionalProperties":true,"required":["scriptPath"],"properties":{"projectFolder":{"type":"string"},"scriptPath":{"type":"string"}}}}
])JSON";

glz::generic Tools()
{
    glz::generic tools;

    if (glz::read_json(tools, std::string(kToolsJson)) || !tools.is_array() ||
        tools.get_array().empty()) {
        std::cerr << "[ERROR] MCP tool catalog JSON failed to parse\n" << std::flush;
        return {};
    }
    return tools;
}

std::string DefaultProjectFolder()
{
    const char* env = std::getenv("RTYPE_PROJECT_FOLDER");
    std::error_code error;

    if (env != nullptr && env[0] != '\0') {
        return env;
    }
    return std::filesystem::current_path(error).string();
}

std::string DefaultEngineRoot()
{
    const char* env = std::getenv("RTYPE_ENGINE_ROOT");

    if (env != nullptr && env[0] != '\0') {
        return env;
    }
#ifdef RTYPE_SOURCE_DIR
    return RTYPE_SOURCE_DIR;
#else
    return {};
#endif
}

void FillDefault(glz::generic::object_t& obj, const char* key, const std::string& value)
{
    const auto it = obj.find(key);
    if (it == obj.end() || (it->second.is_string() && it->second.get_string().empty())) {
        obj[key] = value;
    }
}

glz::generic ErrorObject(int code, const std::string& message)
{
    glz::generic error = glz::generic::object_t{};
    auto& obj = error.get_object();

    obj["code"] = static_cast<double>(code);
    obj["message"] = message;
    return error;
}

glz::generic CallTool(const glz::generic& params)
{
    glz::generic payload = glz::generic::object_t{};
    std::string name;
    std::string encoded;
    AuthorRequest request;
    glz::generic commandResult;
    glz::generic contentItem = glz::generic::object_t{};
    glz::generic content = glz::generic::array_t{};
    glz::generic result = glz::generic::object_t{};
    std::string pretty;
    bool ok = false;
    constexpr glz::opts options{.error_on_unknown_keys = false};

    if (params.is_object()) {
        const auto& obj = params.get_object();
        const auto nameIt = obj.find("name");
        const auto argsIt = obj.find("arguments");
        if (nameIt != obj.end() && nameIt->second.is_string()) {
            name = nameIt->second.get_string();
        }
        if (argsIt != obj.end() && argsIt->second.is_object()) {
            payload = argsIt->second;
        }
    }
    payload.get_object()["command"] = name;
    FillDefault(payload.get_object(), "projectFolder", DefaultProjectFolder());
    FillDefault(payload.get_object(), "engineRoot", DefaultEngineRoot());
    if (glz::write_json(payload, encoded) || glz::read<options>(request, encoded)) {
        commandResult = glz::generic::object_t{};
        commandResult.get_object()["ok"] = false;
        commandResult.get_object()["apiVersion"] = kApiVersion;
        commandResult.get_object()["error"] = "could not read tool arguments";
    } else {
        try {
            commandResult = RunCommand(request);
        } catch (const std::exception& exception) {
            commandResult = glz::generic::object_t{};
            commandResult.get_object()["ok"] = false;
            commandResult.get_object()["apiVersion"] = kApiVersion;
            commandResult.get_object()["error"] = exception.what();
        }
    }
    if (commandResult.is_object()) {
        const auto it = commandResult.get_object().find("ok");
        ok = it != commandResult.get_object().end() && it->second.is_boolean() &&
             it->second.get<bool>();
    }
    if (glz::write<glz::opts{.prettify = true}>(commandResult, pretty)) {
        pretty = "{\"ok\":false,\"error\":\"failed to encode tool result\"}";
        ok = false;
    }
    contentItem.get_object()["type"] = std::string("text");
    contentItem.get_object()["text"] = pretty;
    content.get_array().push_back(std::move(contentItem));
    result.get_object()["content"] = std::move(content);
    result.get_object()["isError"] = !ok;
    return result;
}

glz::generic Handle(const glz::generic::object_t& message, bool& hasId, glz::generic& id)
{
    const auto idIt = message.find("id");
    const auto methodIt = message.find("method");
    std::string method;
    glz::generic params;

    hasId = idIt != message.end() && !idIt->second.is_null();
    if (!hasId) {
        return {};
    }
    id = idIt->second;
    if (methodIt == message.end() || !methodIt->second.is_string()) {
        return ErrorObject(-32600, "Invalid Request");
    }
    method = methodIt->second.get_string();
    if (const auto paramsIt = message.find("params"); paramsIt != message.end()) {
        params = paramsIt->second;
    }
    if (method == "initialize") {
        glz::generic result = glz::generic::object_t{};
        auto& obj = result.get_object();
        glz::generic capabilities = glz::generic::object_t{};
        glz::generic info = glz::generic::object_t{};
        capabilities.get_object()["tools"] = glz::generic::object_t{};
        info.get_object()["name"] = std::string("rtype-author");
        info.get_object()["version"] = std::string("1.0.0");
        obj["protocolVersion"] = std::string(kProtocolVersion);
        obj["capabilities"] = std::move(capabilities);
        obj["serverInfo"] = std::move(info);
        return result;
    }
    if (method == "ping") {
        return glz::generic::object_t{};
    }
    if (method == "tools/list") {
        glz::generic tools = Tools();
        glz::generic result = glz::generic::object_t{};

        if (!tools.is_array()) {
            return ErrorObject(-32603, "tool catalog failed to parse");
        }
        result.get_object()["tools"] = std::move(tools);
        return result;
    }
    if (method == "tools/call") {
        return CallTool(params);
    }
    return ErrorObject(-32601, "Method not found: " + method);
}

bool IsRpcError(const glz::generic& value)
{
    if (!value.is_object()) {
        return false;
    }
    const auto& obj = value.get_object();
    return obj.find("code") != obj.end() && obj.find("message") != obj.end() &&
           obj.find("protocolVersion") == obj.end() && obj.find("content") == obj.end() &&
           obj.find("tools") == obj.end();
}

void Reply(const glz::generic& id, const glz::generic& body)
{
    glz::generic response = glz::generic::object_t{};
    auto& obj = response.get_object();
    std::string json;

    obj["jsonrpc"] = std::string("2.0");
    obj["id"] = id;
    if (IsRpcError(body)) {
        obj["error"] = body;
    } else {
        obj["result"] = body;
    }
    if (glz::write_json(response, json)) {
        json = "{\"jsonrpc\":\"2.0\",\"id\":null,\"error\":{\"code\":-32603,\"message\":\"failed "
               "to encode response\"}}";
    }
    std::cout << json << '\n' << std::flush;
}

} // namespace

const char* ToolCatalogJson()
{
    return kToolsJson;
}

int RunMcpStdio()
{
    std::string line;

    Engine::SetTerminalLogToStderr();
    while (std::getline(std::cin, line)) {
        glz::generic message;
        bool hasId = false;
        glz::generic id;
        glz::generic body;

        if (line.empty()) {
            continue;
        }
        if (glz::read_json(message, line) || !message.is_object()) {
            continue;
        }
        body = Handle(message.get_object(), hasId, id);
        if (!hasId) {
            continue;
        }
        Reply(id, body);
    }
    Engine::ShutdownLogging();
    return 0;
}

} // namespace Author
