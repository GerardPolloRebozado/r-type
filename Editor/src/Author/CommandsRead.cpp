#include "Author/CommandsInternal.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/Animation.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ProjectFile.hpp"
#include "Resources/WaveSchedule.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "UI/UiCatalog.hpp"
#include "UI/UiComponent.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace Author {
namespace {

void PushStringList(glz::generic& out, const std::vector<std::string>& values)
{
    glz::generic array = glz::generic::array_t{};
    auto& items = array.get_array();

    for (const std::string& value : values) {
        items.emplace_back(value);
    }
    out = std::move(array);
}

void AppendComponentIf(Engine::Registry& registry, Engine::Entity entity, const char* name,
                       std::vector<std::string>& names)
{
    if (name == std::string("name") && registry.IsComponentRegistered<Engine::Name>() &&
        registry.HasComponent<Engine::Name>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("position") &&
               registry.IsComponentRegistered<Engine::Position>() &&
               registry.HasComponent<Engine::Position>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("rotation") &&
               registry.IsComponentRegistered<Engine::Rotation>() &&
               registry.HasComponent<Engine::Rotation>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("sprite") &&
               registry.IsComponentRegistered<Engine::SpriteComponent>() &&
               registry.HasComponent<Engine::SpriteComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("shape") &&
               registry.IsComponentRegistered<Engine::ShapeComponent>() &&
               registry.HasComponent<Engine::ShapeComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("text") && registry.IsComponentRegistered<Engine::Text>() &&
               registry.HasComponent<Engine::Text>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("collider") &&
               registry.IsComponentRegistered<Engine::ColliderComponent>() &&
               registry.HasComponent<Engine::ColliderComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("opacity") &&
               registry.IsComponentRegistered<Engine::Opacity>() &&
               registry.HasComponent<Engine::Opacity>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("parent") && registry.IsComponentRegistered<Engine::Parent>() &&
               registry.HasComponent<Engine::Parent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("input") && registry.IsComponentRegistered<Engine::Input>() &&
               registry.HasComponent<Engine::Input>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("audio") &&
               registry.IsComponentRegistered<Engine::AudioSource>() &&
               registry.HasComponent<Engine::AudioSource>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("camera") &&
               registry.IsComponentRegistered<Engine::CameraComponent>() &&
               registry.HasComponent<Engine::CameraComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("script") &&
               registry.IsComponentRegistered<Engine::ScriptComponent>() &&
               registry.HasComponent<Engine::ScriptComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("ui") && registry.IsComponentRegistered<Engine::UiComponent>() &&
               registry.HasComponent<Engine::UiComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("network") &&
               registry.IsComponentRegistered<Engine::NetworkComponent>() &&
               registry.HasComponent<Engine::NetworkComponent>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("animation") &&
               registry.IsComponentRegistered<Engine::Animation>() &&
               registry.HasComponent<Engine::Animation>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("prefab") &&
               registry.IsComponentRegistered<Engine::PrefabInstance>() &&
               registry.HasComponent<Engine::PrefabInstance>(entity)) {
        names.emplace_back(name);
    } else if (name == std::string("connections") &&
               registry.IsComponentRegistered<Engine::SignalConnections>() &&
               registry.HasComponent<Engine::SignalConnections>(entity)) {
        names.emplace_back(name);
    }
}

void ListFiles(const std::filesystem::path& root, const std::string& extension,
               std::vector<std::string>& out)
{
    std::error_code error;
    std::string relative;

    if (!std::filesystem::exists(root, error) || !std::filesystem::is_directory(root, error)) {
        return;
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             root, std::filesystem::directory_options::skip_permission_denied, error)) {
        if (error || !entry.is_regular_file(error)) {
            continue;
        }
        if (!extension.empty() && entry.path().extension() != extension) {
            continue;
        }
        relative = std::filesystem::relative(entry.path(), root.parent_path(), error).string();
        if (error) {
            relative = entry.path().filename().string();
        }
        std::replace(relative.begin(), relative.end(), '\\', '/');
        out.push_back(relative);
    }
    std::sort(out.begin(), out.end());
}

bool LooksSafeTopic(const std::string& topic)
{
    if (topic.empty() || topic.find("..") != std::string::npos) {
        return false;
    }
    for (char letter : topic) {
        if (std::isalnum(static_cast<unsigned char>(letter)) == 0 && letter != '/' &&
            letter != '-' && letter != '_') {
            return false;
        }
    }
    return true;
}

glz::generic ColorArray(Color color)
{
    glz::generic array = glz::generic::array_t{};
    auto& items = array.get_array();

    items.emplace_back(static_cast<double>(color.r));
    items.emplace_back(static_cast<double>(color.g));
    items.emplace_back(static_cast<double>(color.b));
    items.emplace_back(static_cast<double>(color.a));
    return array;
}

const char* ShapeTypeName(Engine::ShapeType type)
{
    if (type == Engine::ShapeType::CIRCLE) {
        return "circle";
    }
    if (type == Engine::ShapeType::POLYGON) {
        return "polygon";
    }
    if (type == Engine::ShapeType::LINE) {
        return "line";
    }
    return "rectangle";
}

const char* FormationName(Engine::WavePatternType type)
{
    if (type == Engine::WavePatternType::VFormation) {
        return "vformation";
    }
    if (type == Engine::WavePatternType::Sine) {
        return "sine";
    }
    if (type == Engine::WavePatternType::Grid) {
        return "grid";
    }
    return "line";
}

bool ReadComponent(Engine::Registry& registry, Engine::Entity entity, const std::string& component,
                   glz::generic& fields)
{
    fields = glz::generic::object_t{};
    auto& obj = fields.get_object();

    if (component == "position") {
        if (!registry.IsComponentRegistered<Engine::Position>() ||
            !registry.HasComponent<Engine::Position>(entity)) {
            return false;
        }
        const Engine::Position& position = registry.RequireComponent<Engine::Position>(entity);
        obj["x"] = position.x;
        obj["y"] = position.y;
        obj["z"] = static_cast<double>(position.z);
        obj["relative"] = position.relative;
        return true;
    }
    if (component == "rotation") {
        if (!registry.IsComponentRegistered<Engine::Rotation>() ||
            !registry.HasComponent<Engine::Rotation>(entity)) {
            return false;
        }
        obj["angle"] = registry.RequireComponent<Engine::Rotation>(entity).angle;
        return true;
    }
    if (component == "name") {
        if (!registry.IsComponentRegistered<Engine::Name>() ||
            !registry.HasComponent<Engine::Name>(entity)) {
            return false;
        }
        obj["value"] = registry.RequireComponent<Engine::Name>(entity).value;
        return true;
    }
    if (component == "sprite") {
        if (!registry.IsComponentRegistered<Engine::SpriteComponent>() ||
            !registry.HasComponent<Engine::SpriteComponent>(entity)) {
            return false;
        }
        const Engine::SpriteComponent& sprite =
            registry.RequireComponent<Engine::SpriteComponent>(entity);
        obj["assetPath"] = sprite.assetPath;
        obj["offsetX"] = sprite.offsetX;
        obj["offsetY"] = sprite.offsetY;
        obj["scaleX"] = sprite.scaleX;
        obj["scaleY"] = sprite.scaleY;
        obj["flipX"] = sprite.flipX;
        obj["flipY"] = sprite.flipY;
        return true;
    }
    if (component == "collider") {
        if (!registry.IsComponentRegistered<Engine::ColliderComponent>() ||
            !registry.HasComponent<Engine::ColliderComponent>(entity)) {
            return false;
        }
        const Engine::ColliderComponent& collider =
            registry.RequireComponent<Engine::ColliderComponent>(entity);
        obj["enabled"] = collider.enabled;
        obj["isTrigger"] = collider.isTrigger;
        obj["isStatic"] = collider.isStatic;
        obj["layer"] = static_cast<double>(collider.layer);
        obj["mask"] = static_cast<double>(collider.mask);
        return true;
    }
    if (component == "shape") {
        if (!registry.IsComponentRegistered<Engine::ShapeComponent>() ||
            !registry.HasComponent<Engine::ShapeComponent>(entity)) {
            return false;
        }
        const Engine::ShapeComponent& shape = registry.RequireComponent<Engine::ShapeComponent>(entity);
        obj["type"] = std::string(ShapeTypeName(shape.type));
        obj["width"] = shape.width;
        obj["height"] = shape.height;
        obj["radius"] = shape.radius;
        obj["sides"] = static_cast<double>(shape.sides);
        obj["filled"] = shape.filled;
        obj["color"] = ColorArray(shape.color);
        return true;
    }
    if (component == "text") {
        if (!registry.IsComponentRegistered<Engine::Text>() ||
            !registry.HasComponent<Engine::Text>(entity)) {
            return false;
        }
        const Engine::Text& text = registry.RequireComponent<Engine::Text>(entity);
        obj["text"] = text.text;
        obj["fontsize"] = text.fontsize;
        obj["spacing"] = text.spacing;
        obj["color"] = ColorArray(text.color);
        return true;
    }
    if (component == "opacity") {
        if (!registry.IsComponentRegistered<Engine::Opacity>() ||
            !registry.HasComponent<Engine::Opacity>(entity)) {
            return false;
        }
        const Engine::Opacity& opacity = registry.RequireComponent<Engine::Opacity>(entity);
        obj["alpha"] = opacity.alpha;
        obj["inheritToChildren"] = opacity.inheritToChildren;
        return true;
    }
    if (component == "camera") {
        if (!registry.IsComponentRegistered<Engine::CameraComponent>() ||
            !registry.HasComponent<Engine::CameraComponent>(entity)) {
            return false;
        }
        const Engine::CameraComponent& camera =
            registry.RequireComponent<Engine::CameraComponent>(entity);
        obj["zoom"] = camera.zoom;
        obj["primary"] = camera.primary;
        obj["followSpeed"] = camera.followSpeed;
        return true;
    }
    if (component == "audio") {
        if (!registry.IsComponentRegistered<Engine::AudioSource>() ||
            !registry.HasComponent<Engine::AudioSource>(entity)) {
            return false;
        }
        const Engine::AudioSource& audio = registry.RequireComponent<Engine::AudioSource>(entity);
        obj["assetId"] = audio.assetId;
        obj["isMusic"] = audio.isMusic;
        obj["loop"] = audio.loop;
        obj["volume"] = audio.volume;
        return true;
    }
    if (component == "input") {
        if (!registry.IsComponentRegistered<Engine::Input>() ||
            !registry.HasComponent<Engine::Input>(entity)) {
            return false;
        }
        const Engine::Input& input = registry.RequireComponent<Engine::Input>(entity);
        obj["playerIdx"] = static_cast<double>(input.playerIdx);
        obj["isActive"] = input.isActive;
        return true;
    }
    if (component == "network") {
        if (!registry.IsComponentRegistered<Engine::NetworkComponent>() ||
            !registry.HasComponent<Engine::NetworkComponent>(entity)) {
            return false;
        }
        const Engine::NetworkComponent& network =
            registry.RequireComponent<Engine::NetworkComponent>(entity);
        obj["authority"] = network.authority;
        obj["syncTransform"] = network.syncTransform;
        obj["syncSprite"] = network.syncSprite;
        obj["prefabPath"] = network.prefabPath;
        return true;
    }
    return false;
}

} // namespace

std::string FieldTypeName(Engine::UiFieldType type)
{
    if (type == Engine::UiFieldType::Bool) {
        return "bool";
    }
    if (type == Engine::UiFieldType::Number) {
        return "number";
    }
    if (type == Engine::UiFieldType::Enum) {
        return "enum";
    }
    if (type == Engine::UiFieldType::StringList) {
        return "stringList";
    }
    if (type == Engine::UiFieldType::Color) {
        return "color";
    }
    return "string";
}

std::vector<std::string> ComponentNames(Engine::Registry& registry, Engine::Entity entity)
{
    std::vector<std::string> names;
    static const char* kKeys[] = {"name",    "position", "rotation", "sprite",  "shape",
                                  "text",    "collider", "opacity",  "parent",  "input",
                                  "audio",   "camera",   "script",   "ui",      "network",
                                  "animation", "prefab", "connections"};

    for (const char* key : kKeys) {
        AppendComponentIf(registry, entity, key, names);
    }
    return names;
}

std::filesystem::path FindEngineRoot(const std::string& engineRoot)
{
    const char* env = std::getenv("RTYPE_ENGINE_ROOT");
    std::error_code error;
    std::filesystem::path candidate;
    std::filesystem::path cursor;

    if (!engineRoot.empty()) {
        candidate = std::filesystem::absolute(engineRoot, error).lexically_normal();
        if (std::filesystem::exists(candidate / "Documentation" / "docs" / "lua" / "index.md")) {
            return candidate;
        }
    }
    if (env != nullptr && env[0] != '\0') {
        candidate = std::filesystem::absolute(env, error).lexically_normal();
        if (std::filesystem::exists(candidate / "Documentation" / "docs" / "lua" / "index.md")) {
            return candidate;
        }
    }
    cursor = std::filesystem::current_path(error);
    while (!cursor.empty() && cursor != cursor.root_path()) {
        if (std::filesystem::exists(cursor / "Documentation" / "docs" / "lua" / "index.md")) {
            return cursor;
        }
        if (!cursor.has_parent_path()) {
            break;
        }
        cursor = cursor.parent_path();
    }
    return {};
}

std::string ReadLuaDoc(const std::filesystem::path& engineRoot, const std::string& topic)
{
    std::string page = topic.empty() ? "index" : topic;
    std::filesystem::path path;
    std::ifstream file;
    std::string content;

    if (!LooksSafeTopic(page)) {
        return {};
    }
    path = engineRoot / "Documentation" / "docs" / "lua" / (page + ".md");
    file.open(path);
    if (!file.is_open()) {
        return {};
    }
    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return content;
}

glz::generic ProjectSummary(const AuthorRequest& request)
{
    std::filesystem::path project = ResolveProjectFolder(request.projectFolder);
    Serialization::ProjectFile file;
    glz::generic result;
    auto* obj = static_cast<glz::generic::object_t*>(nullptr);
    std::vector<std::string> scenes;
    std::vector<std::string> prefabs;
    std::error_code error;
    Session session;
    std::string loadError;

    result = MakeResult(true, "");
    obj = &result.get_object();
    if (project.empty() || !std::filesystem::is_directory(project, error)) {
        return MakeResult(false, "projectFolder is not a directory");
    }
    Serialization::LoadProjectFile(project, file);
    (*obj)["projectFolder"] = project.string();
    (*obj)["startupScene"] = file.startupScene;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(
             project, std::filesystem::directory_options::skip_permission_denied, error)) {
        if (error || !entry.is_regular_file(error)) {
            continue;
        }
        if (entry.path().extension() == ".scene") {
            scenes.push_back(
                std::filesystem::relative(entry.path(), project, error).string());
        }
    }
    std::sort(scenes.begin(), scenes.end());
    ListFiles(project / "Prefabs", ".prefab", prefabs);
    PushStringList((*obj)["scenes"], scenes);
    PushStringList((*obj)["prefabs"], prefabs);
    loadError = session.Open(project, ResolveScenePath(project, request.scenePath));
    if (loadError.empty()) {
        (*obj)["scenePath"] = session.ScenePath().string();
        (*obj)["entityCount"] =
            static_cast<double>(session.Registry().GetEntityCount() > 0
                                    ? session.Registry().GetEntityCount() - 1
                                    : 0);
        (*obj)["waveCount"] = static_cast<double>(session.Config().m_Waves.size());
    } else {
        (*obj)["sceneError"] = loadError;
    }
    return result;
}

glz::generic SceneOutline(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    glz::generic result;
    auto* obj = static_cast<glz::generic::object_t*>(nullptr);
    Engine::Registry* registry = nullptr;
    glz::generic entities = glz::generic::array_t{};
    glz::generic waves = glz::generic::array_t{};
    std::size_t index = 0;
    Engine::Entity entity = 0;
    std::optional<Engine::Entity> parent;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    result = MakeResult(true, "");
    obj = &result.get_object();
    registry = &session.Registry();
    (*obj)["scenePath"] = session.ScenePath().string();
    (*obj)["projectFolder"] = session.ProjectFolder().string();
    for (index = 1; index < registry->GetEntityCount(); ++index) {
        entity = Engine::Entity{index};
        if (!registry->IsEntityAlive(entity)) {
            continue;
        }
        glz::generic row = glz::generic::object_t{};
        auto& rowObj = row.get_object();
        rowObj["id"] = static_cast<double>(entity);
        rowObj["name"] = EntityDisplayName(*registry, entity);
        parent = std::nullopt;
        if (registry->IsComponentRegistered<Engine::Parent>() &&
            registry->HasComponent<Engine::Parent>(entity)) {
            parent = registry->RequireComponent<Engine::Parent>(entity).parent;
        }
        if (parent.has_value()) {
            rowObj["parent"] = static_cast<double>(parent.value());
            rowObj["parentName"] = EntityDisplayName(*registry, parent.value());
        }
        PushStringList(rowObj["components"], ComponentNames(*registry, entity));
        if (registry->IsComponentRegistered<Engine::Position>() &&
            registry->HasComponent<Engine::Position>(entity)) {
            const Engine::Position& position = registry->RequireComponent<Engine::Position>(entity);
            rowObj["x"] = position.x;
            rowObj["y"] = position.y;
            rowObj["z"] = static_cast<double>(position.z);
        }
        if (registry->IsComponentRegistered<Engine::ScriptComponent>() &&
            registry->HasComponent<Engine::ScriptComponent>(entity)) {
            rowObj["scriptPath"] =
                registry->RequireComponent<Engine::ScriptComponent>(entity).scriptPath;
        }
        if (registry->IsComponentRegistered<Engine::UiComponent>() &&
            registry->HasComponent<Engine::UiComponent>(entity)) {
            const Engine::UiComponent& ui = registry->RequireComponent<Engine::UiComponent>(entity);
            rowObj["uiTemplate"] = ui.templateName;
            rowObj["page"] = Engine::UiPageOf(*registry, entity);
            rowObj["order"] = static_cast<double>(ui.order);
        }
        if (registry->IsComponentRegistered<Engine::PrefabInstance>() &&
            registry->HasComponent<Engine::PrefabInstance>(entity)) {
            rowObj["prefabPath"] =
                registry->RequireComponent<Engine::PrefabInstance>(entity).prefabPath;
        }
        entities.get_array().push_back(std::move(row));
    }
    (*obj)["entities"] = std::move(entities);
    for (index = 0; index < session.Config().m_Waves.size(); ++index) {
        const Engine::WaveDefinition& wave = session.Config().m_Waves[index];
        glz::generic row = glz::generic::object_t{};
        auto& rowObj = row.get_object();
        rowObj["index"] = static_cast<double>(index);
        rowObj["name"] = wave.name;
        rowObj["triggerX"] = wave.triggerX;
        rowObj["prefabPath"] = wave.prefabPath;
        rowObj["formation"] = std::string(FormationName(wave.formation));
        rowObj["count"] = static_cast<double>(wave.count);
        rowObj["spacing"] = wave.spacing;
        rowObj["baseY"] = wave.baseY;
        rowObj["spawnOffsetX"] = wave.spawnOffsetX;
        rowObj["attachParent"] = wave.attachParent;
        rowObj["groupScriptPath"] = wave.groupScriptPath;
        waves.get_array().push_back(std::move(row));
    }
    (*obj)["waves"] = std::move(waves);
    return result;
}

glz::generic GetComponentCmd(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> entity;
    glz::generic result;
    glz::generic fields;
    static const char* kReadable[] = {"position", "rotation", "name",    "sprite", "collider",
                                      "shape",    "text",     "opacity", "camera", "audio",
                                      "input",    "network"};
    std::size_t index = 0;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = FindEntity(session.Registry(), request.entity);
    if (!entity.has_value()) {
        return MakeResult(false, "entity not found: " + request.entity);
    }
    result = MakeResult(true, "");
    result.get_object()["id"] = static_cast<double>(entity.value());
    result.get_object()["name"] = EntityDisplayName(session.Registry(), entity.value());
    if (request.component.empty()) {
        glz::generic all = glz::generic::object_t{};
        for (index = 0; index < sizeof(kReadable) / sizeof(kReadable[0]); ++index) {
            if (ReadComponent(session.Registry(), entity.value(), kReadable[index], fields)) {
                all.get_object()[kReadable[index]] = std::move(fields);
            }
        }
        result.get_object()["components"] = std::move(all);
        return result;
    }
    for (index = 0; index < sizeof(kReadable) / sizeof(kReadable[0]); ++index) {
        if (request.component == kReadable[index]) {
            break;
        }
    }
    if (index == sizeof(kReadable) / sizeof(kReadable[0])) {
        return MakeResult(false, "unknown component '" + request.component + "'. allowed: " +
                                     AllowedComponents());
    }
    if (!ReadComponent(session.Registry(), entity.value(), request.component, fields)) {
        return MakeResult(false, "entity has no " + request.component + " component");
    }
    result.get_object()["component"] = request.component;
    result.get_object()["fields"] = std::move(fields);
    return result;
}

glz::generic ListUiTemplates(const AuthorRequest&)
{
    glz::generic result = MakeResult(true, "");
    auto& obj = result.get_object();
    glz::generic templates = glz::generic::array_t{};
    int index = 0;
    int fieldIndex = 0;

    for (index = 0; index < Engine::UiTemplateCount(); ++index) {
        const Engine::UiTemplateInfo& info = Engine::UiTemplates()[index];
        glz::generic row = glz::generic::object_t{};
        auto& rowObj = row.get_object();
        glz::generic fields = glz::generic::array_t{};
        rowObj["id"] = std::string(info.id);
        rowObj["label"] = std::string(info.label);
        rowObj["group"] = std::string(info.group);
        for (fieldIndex = 0; fieldIndex < info.fieldCount; ++fieldIndex) {
            const Engine::UiField& field = info.fields[fieldIndex];
            glz::generic fieldJson = glz::generic::object_t{};
            auto& fieldObj = fieldJson.get_object();
            fieldObj["key"] = std::string(field.key);
            fieldObj["label"] = std::string(field.label);
            fieldObj["type"] = FieldTypeName(field.type);
            fieldObj["integer"] = field.integer;
            fieldObj["boolDefault"] = field.boolDefault;
            fieldObj["numberDefault"] = field.numberDefault;
            fieldObj["stringDefault"] = std::string(field.stringDefault ? field.stringDefault : "");
            if (field.options != nullptr && field.optionCount > 0) {
                glz::generic options = glz::generic::array_t{};
                int option = 0;
                for (option = 0; option < field.optionCount; ++option) {
                    options.get_array().emplace_back(std::string(field.options[option]));
                }
                fieldObj["options"] = std::move(options);
            }
            fields.get_array().push_back(std::move(fieldJson));
        }
        rowObj["fields"] = std::move(fields);
        templates.get_array().push_back(std::move(row));
    }
    obj["templates"] = std::move(templates);
    return result;
}

glz::generic ListAssets(const AuthorRequest& request)
{
    std::filesystem::path project = ResolveProjectFolder(request.projectFolder);
    glz::generic result;
    std::vector<std::string> assets;
    std::error_code error;

    if (project.empty() || !std::filesystem::is_directory(project, error)) {
        return MakeResult(false, "projectFolder is not a directory");
    }
    result = MakeResult(true, "");
    ListFiles(project / "assets", "", assets);
    PushStringList(result.get_object()["assets"], assets);
    result.get_object()["projectFolder"] = project.string();
    return result;
}

glz::generic ListPrefabs(const AuthorRequest& request)
{
    std::filesystem::path project = ResolveProjectFolder(request.projectFolder);
    glz::generic result;
    std::vector<std::string> prefabs;
    std::error_code error;

    if (project.empty() || !std::filesystem::is_directory(project, error)) {
        return MakeResult(false, "projectFolder is not a directory");
    }
    result = MakeResult(true, "");
    ListFiles(project / "Prefabs", ".prefab", prefabs);
    PushStringList(result.get_object()["prefabs"], prefabs);
    result.get_object()["projectFolder"] = project.string();
    return result;
}

glz::generic LuaApi(const AuthorRequest& request)
{
    std::filesystem::path root = FindEngineRoot(request.engineRoot);
    glz::generic result;
    std::string content;
    std::string topic = request.topic.empty() ? "index" : request.topic;

    if (root.empty()) {
        return MakeResult(false, "engine root not found (set engineRoot or RTYPE_ENGINE_ROOT)");
    }
    content = ReadLuaDoc(root, topic);
    if (content.empty()) {
        return MakeResult(false, "lua doc page not found: " + topic);
    }
    result = MakeResult(true, "");
    result.get_object()["topic"] = topic;
    result.get_object()["path"] = "Documentation/docs/lua/" + topic + ".md";
    result.get_object()["markdown"] = content;
    return result;
}

glz::generic ValidateScene(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    glz::generic result;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    result = MakeResult(true, "");
    result.get_object()["scenePath"] = session.ScenePath().string();
    return result;
}

} // namespace Author
