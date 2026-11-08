#include "Author/CommandsInternal.hpp"
#include "Assets/ProjectPaths.hpp"
#include "GlazeSerializer.hpp"
#include "Systems/HierarchySystem.hpp"
#include "Components/Animation.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "Resources/WaveSchedule.hpp"
#include "RuntimePrefab.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "UI/UiCatalog.hpp"
#include "UI/UiComponent.hpp"
#include "Utils/WavePatterns.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sol/sol.hpp>
#include <sstream>
#include <unordered_map>

namespace Author {
namespace {

std::string SanitizeFileStem(const std::string& name)
{
    std::string stem;
    char letter = 0;

    for (char raw : name) {
        letter = raw;
        if (std::isalnum(static_cast<unsigned char>(letter)) != 0 || letter == '_' ||
            letter == '-') {
            stem.push_back(letter);
        } else if (letter == ' ') {
            stem.push_back('_');
        }
    }
    if (stem.empty()) {
        stem = "script";
    }
    return stem;
}

std::string IndentBody(const std::string& body, const std::string& fallback)
{
    std::istringstream stream(body);
    std::string line;
    std::string out;

    if (body.empty()) {
        return "    " + fallback + "\n";
    }
    while (std::getline(stream, line)) {
        out += "    " + line + "\n";
    }
    return out;
}

const char* HookArgs(const std::string& name)
{
    if (name == "onUpdate") {
        return "entity, dt";
    }
    if (name == "onCollisionEnter" || name == "onCollisionStay" || name == "onCollisionExit") {
        return "entity, other";
    }
    return "entity";
}

Engine::UiActionType ParseActionType(const std::string& raw)
{
    std::string value = raw;
    std::size_t index = 0;

    for (index = 0; index < value.size(); ++index) {
        value[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[index])));
    }
    if (value == "gotopage" || value == "go to page") {
        return Engine::UiActionType::GoToPage;
    }
    if (value == "back") {
        return Engine::UiActionType::Back;
    }
    if (value == "callmethod" || value == "call method") {
        return Engine::UiActionType::CallMethod;
    }
    if (value == "loadscene" || value == "load scene") {
        return Engine::UiActionType::LoadScene;
    }
    if (value == "quit") {
        return Engine::UiActionType::Quit;
    }
    return Engine::UiActionType::None;
}

Engine::WavePatternType ParseFormation(const std::string& raw)
{
    std::string value = raw;
    std::size_t index = 0;

    for (index = 0; index < value.size(); ++index) {
        if (value[index] == ' ') {
            continue;
        }
        value[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[index])));
    }
    value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
    if (value == "vformation" || value == "v") {
        return Engine::WavePatternType::VFormation;
    }
    if (value == "sine") {
        return Engine::WavePatternType::Sine;
    }
    if (value == "grid") {
        return Engine::WavePatternType::Grid;
    }
    return Engine::WavePatternType::Line;
}

Engine::ShapeType ParseShapeType(const std::string& raw)
{
    std::string value = raw;
    std::size_t index = 0;

    for (index = 0; index < value.size(); ++index) {
        value[index] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[index])));
    }
    if (value == "CIRCLE") {
        return Engine::ShapeType::CIRCLE;
    }
    if (value == "POLYGON") {
        return Engine::ShapeType::POLYGON;
    }
    if (value == "LINE") {
        return Engine::ShapeType::LINE;
    }
    return Engine::ShapeType::RECTANGLE;
}

Color ParseColor(const glz::generic& value, Color fallback)
{
    Color color = fallback;

    if (value.is_array() && value.get_array().size() >= 3) {
        const auto& items = value.get_array();
        color.r = static_cast<unsigned char>(items[0].get<double>());
        color.g = static_cast<unsigned char>(items[1].get<double>());
        color.b = static_cast<unsigned char>(items[2].get<double>());
        color.a = items.size() > 3 ? static_cast<unsigned char>(items[3].get<double>())
                                   : static_cast<unsigned char>(255);
        return color;
    }
    if (value.is_object()) {
        const auto& obj = value.get_object();
        color.r = static_cast<unsigned char>(GenericNumber(obj, "r", fallback.r));
        color.g = static_cast<unsigned char>(GenericNumber(obj, "g", fallback.g));
        color.b = static_cast<unsigned char>(GenericNumber(obj, "b", fallback.b));
        color.a = static_cast<unsigned char>(GenericNumber(obj, "a", fallback.a));
    }
    return color;
}

std::optional<Engine::Entity> FindScreen(Engine::Registry& registry, const std::string& key)
{
    std::optional<Engine::Entity> page;
    std::optional<Engine::Entity> entity;

    page = Engine::FindUiPageScreen(registry, key);
    if (page.has_value()) {
        return page;
    }
    entity = FindEntity(registry, key);
    if (entity.has_value() && Engine::IsUiScreenEntity(registry, entity.value())) {
        return entity;
    }
    return std::nullopt;
}

std::string WriteScriptFile(const std::filesystem::path& path, const std::string& entityName,
                            const glz::generic& hooks)
{
    std::ofstream out(path);
    std::unordered_map<std::string, std::string> bodies;
    static const char* kCore[] = {"onCreate", "onUpdate", "onDestroy"};
    const char* hook = nullptr;
    int index = 0;

    if (!out) {
        return "failed to write " + path.string();
    }
    if (GenericIsObject(hooks)) {
        for (const auto& [name, value] : hooks.get_object()) {
            if (value.is_string()) {
                bodies[name] = value.get_string();
            }
        }
    }
    out << "-- " << path.filename().string() << "\n";
    out << "-- Attached to: " << entityName << "\n\n";
    out << "local Script = {}\n\n";
    out << "function Script:onCreate(entity)\n";
    out << "    self.entity = entity\n";
    if (bodies.contains("onCreate") && !bodies["onCreate"].empty()) {
        out << IndentBody(bodies["onCreate"], "");
    } else {
        out << "    log.info(\"[\" .. tostring(entity) .. \"] Script created\")\n";
    }
    out << "end\n\n";
    out << "function Script:onUpdate(entity, dt)\n";
    out << IndentBody(bodies.contains("onUpdate") ? bodies["onUpdate"] : "",
                      "-- Add your per-frame logic here");
    out << "end\n\n";
    out << "function Script:onDestroy(entity)\n";
    if (bodies.contains("onDestroy") && !bodies["onDestroy"].empty()) {
        out << IndentBody(bodies["onDestroy"], "");
    } else {
        out << "    log.info(\"[\" .. tostring(entity) .. \"] Script destroyed\")\n";
    }
    out << "end\n";
    for (const auto& [name, body] : bodies) {
        bool core = false;
        for (index = 0; index < 3; ++index) {
            if (name == kCore[index]) {
                core = true;
            }
        }
        if (core) {
            continue;
        }
        hook = HookArgs(name);
        out << "\nfunction Script:" << name << "(" << hook << ")\n";
        out << IndentBody(body, "--");
        out << "end\n";
    }
    out << "\nreturn Script\n";
    return {};
}

std::string SetPosition(Engine::Registry& registry, Engine::Entity entity,
                        const glz::generic::object_t& fields)
{
    Engine::Position position = registry.RequireComponent<Engine::Position>(entity);

    if (GenericHas(fields, "x")) {
        position.x = static_cast<float>(GenericNumber(fields, "x", position.x));
    }
    if (GenericHas(fields, "y")) {
        position.y = static_cast<float>(GenericNumber(fields, "y", position.y));
    }
    if (GenericHas(fields, "z")) {
        position.z = static_cast<int>(GenericNumber(fields, "z", position.z));
    }
    if (GenericHas(fields, "relative")) {
        position.relative = GenericBool(fields, "relative", position.relative);
    }
    position.prevX = position.x;
    position.prevY = position.y;
    position.prevZ = position.z;
    position.moved = false;
    registry.RequireComponent<Engine::Position>(entity) = position;
    return {};
}

std::string SetSprite(Engine::Registry& registry, Engine::Entity entity,
                      const glz::generic::object_t& fields)
{
    Engine::SpriteComponent sprite;

    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        sprite = registry.RequireComponent<Engine::SpriteComponent>(entity);
    }
    if (GenericHas(fields, "assetPath")) {
        sprite.assetPath = GenericString(fields, "assetPath", sprite.assetPath);
    }
    if (sprite.assetPath.empty()) {
        return "sprite.assetPath is required";
    }
    if (GenericHas(fields, "offsetX")) {
        sprite.offsetX = static_cast<float>(GenericNumber(fields, "offsetX", sprite.offsetX));
    }
    if (GenericHas(fields, "offsetY")) {
        sprite.offsetY = static_cast<float>(GenericNumber(fields, "offsetY", sprite.offsetY));
    }
    if (GenericHas(fields, "scaleX")) {
        sprite.scaleX = static_cast<float>(GenericNumber(fields, "scaleX", sprite.scaleX));
    }
    if (GenericHas(fields, "scaleY")) {
        sprite.scaleY = static_cast<float>(GenericNumber(fields, "scaleY", sprite.scaleY));
    }
    if (GenericHas(fields, "flipX")) {
        sprite.flipX = GenericBool(fields, "flipX", sprite.flipX);
    }
    if (GenericHas(fields, "flipY")) {
        sprite.flipY = GenericBool(fields, "flipY", sprite.flipY);
    }
    if (registry.HasComponent<Engine::SpriteComponent>(entity)) {
        registry.RequireComponent<Engine::SpriteComponent>(entity) = sprite;
    } else {
        registry.AddComponent(entity, sprite);
    }
    return {};
}

std::string SetCollider(Engine::Registry& registry, Engine::Entity entity,
                        const glz::generic::object_t& fields)
{
    Engine::ColliderComponent collider;

    if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
        collider = registry.RequireComponent<Engine::ColliderComponent>(entity);
    }
    if (GenericHas(fields, "enabled")) {
        collider.enabled = GenericBool(fields, "enabled", collider.enabled);
    }
    if (GenericHas(fields, "isTrigger")) {
        collider.isTrigger = GenericBool(fields, "isTrigger", collider.isTrigger);
    }
    if (GenericHas(fields, "isStatic")) {
        collider.isStatic = GenericBool(fields, "isStatic", collider.isStatic);
    }
    if (GenericHas(fields, "layer")) {
        collider.layer = static_cast<int>(GenericNumber(fields, "layer", collider.layer));
    }
    if (GenericHas(fields, "mask")) {
        collider.mask = static_cast<std::uint32_t>(GenericNumber(fields, "mask", collider.mask));
    }
    if (registry.HasComponent<Engine::ColliderComponent>(entity)) {
        registry.RequireComponent<Engine::ColliderComponent>(entity) = collider;
    } else {
        registry.AddComponent(entity, collider);
    }
    return {};
}

std::string SetShape(Engine::Registry& registry, Engine::Entity entity,
                     const glz::generic::object_t& fields)
{
    Engine::ShapeComponent shape;

    if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
        shape = registry.RequireComponent<Engine::ShapeComponent>(entity);
    }
    if (GenericHas(fields, "type")) {
        shape.type = ParseShapeType(GenericString(fields, "type"));
    }
    if (GenericHas(fields, "width")) {
        shape.width = static_cast<float>(GenericNumber(fields, "width", shape.width));
    }
    if (GenericHas(fields, "height")) {
        shape.height = static_cast<float>(GenericNumber(fields, "height", shape.height));
    }
    if (GenericHas(fields, "radius")) {
        shape.radius = static_cast<float>(GenericNumber(fields, "radius", shape.radius));
    }
    if (GenericHas(fields, "sides")) {
        shape.sides = static_cast<int>(GenericNumber(fields, "sides", shape.sides));
    }
    if (GenericHas(fields, "filled")) {
        shape.filled = GenericBool(fields, "filled", shape.filled);
    }
    if (GenericHas(fields, "color")) {
        shape.color = ParseColor(fields.at("color"), shape.color);
    }
    if (registry.HasComponent<Engine::ShapeComponent>(entity)) {
        registry.RequireComponent<Engine::ShapeComponent>(entity) = shape;
    } else {
        registry.AddComponent(entity, shape);
    }
    return {};
}

std::string SetText(Engine::Registry& registry, Engine::Entity entity,
                    const glz::generic::object_t& fields)
{
    Engine::Text text;

    if (registry.HasComponent<Engine::Text>(entity)) {
        text = registry.RequireComponent<Engine::Text>(entity);
    }
    if (GenericHas(fields, "text")) {
        text.text = GenericString(fields, "text", text.text);
    }
    if (GenericHas(fields, "fontsize")) {
        text.fontsize = static_cast<float>(GenericNumber(fields, "fontsize", text.fontsize));
    }
    if (GenericHas(fields, "spacing")) {
        text.spacing = static_cast<float>(GenericNumber(fields, "spacing", text.spacing));
    }
    if (GenericHas(fields, "color")) {
        text.color = ParseColor(fields.at("color"), text.color);
    }
    if (registry.HasComponent<Engine::Text>(entity)) {
        registry.RequireComponent<Engine::Text>(entity) = text;
    } else {
        registry.AddComponent(entity, text);
    }
    return {};
}

std::string SetOpacity(Engine::Registry& registry, Engine::Entity entity,
                       const glz::generic::object_t& fields)
{
    Engine::Opacity opacity;

    if (registry.HasComponent<Engine::Opacity>(entity)) {
        opacity = registry.RequireComponent<Engine::Opacity>(entity);
    }
    if (GenericHas(fields, "alpha")) {
        opacity.alpha = static_cast<float>(GenericNumber(fields, "alpha", opacity.alpha));
    }
    if (GenericHas(fields, "inheritToChildren")) {
        opacity.inheritToChildren =
            GenericBool(fields, "inheritToChildren", opacity.inheritToChildren);
    }
    opacity = Engine::Opacity::Create(opacity.alpha, opacity.inheritToChildren);
    if (registry.HasComponent<Engine::Opacity>(entity)) {
        registry.RequireComponent<Engine::Opacity>(entity) = opacity;
    } else {
        registry.AddComponent(entity, opacity);
    }
    return {};
}

std::string SetCamera(Engine::Registry& registry, Engine::Entity entity,
                      const glz::generic::object_t& fields)
{
    Engine::CameraComponent camera;

    if (registry.HasComponent<Engine::CameraComponent>(entity)) {
        camera = registry.RequireComponent<Engine::CameraComponent>(entity);
    }
    if (GenericHas(fields, "zoom")) {
        camera.zoom = static_cast<float>(GenericNumber(fields, "zoom", camera.zoom));
    }
    if (GenericHas(fields, "primary")) {
        camera.primary = GenericBool(fields, "primary", camera.primary);
    }
    if (GenericHas(fields, "followSpeed")) {
        camera.followSpeed =
            static_cast<float>(GenericNumber(fields, "followSpeed", camera.followSpeed));
    }
    if (registry.HasComponent<Engine::CameraComponent>(entity)) {
        registry.RequireComponent<Engine::CameraComponent>(entity) = camera;
    } else {
        registry.AddComponent(entity, camera);
    }
    return {};
}

std::string SetAudio(Engine::Registry& registry, Engine::Entity entity,
                     const glz::generic::object_t& fields)
{
    Engine::AudioSource audio;

    if (registry.HasComponent<Engine::AudioSource>(entity)) {
        audio = registry.RequireComponent<Engine::AudioSource>(entity);
    }
    if (GenericHas(fields, "assetId")) {
        audio.assetId = GenericString(fields, "assetId", audio.assetId);
    }
    if (GenericHas(fields, "isMusic")) {
        audio.isMusic = GenericBool(fields, "isMusic", audio.isMusic);
    }
    if (GenericHas(fields, "loop")) {
        audio.loop = GenericBool(fields, "loop", audio.loop);
    }
    if (GenericHas(fields, "volume")) {
        audio.volume = static_cast<float>(GenericNumber(fields, "volume", audio.volume));
    }
    if (registry.HasComponent<Engine::AudioSource>(entity)) {
        registry.RequireComponent<Engine::AudioSource>(entity) = audio;
    } else {
        registry.AddComponent(entity, audio);
    }
    return {};
}

std::string SetInput(Engine::Registry& registry, Engine::Entity entity,
                     const glz::generic::object_t& fields)
{
    Engine::Input input;

    if (registry.HasComponent<Engine::Input>(entity)) {
        input = registry.RequireComponent<Engine::Input>(entity);
    }
    if (GenericHas(fields, "playerIdx")) {
        input.playerIdx = static_cast<int>(GenericNumber(fields, "playerIdx", input.playerIdx));
    }
    if (GenericHas(fields, "isActive")) {
        input.isActive = GenericBool(fields, "isActive", input.isActive);
    }
    if (registry.HasComponent<Engine::Input>(entity)) {
        registry.RequireComponent<Engine::Input>(entity) = input;
    } else {
        registry.AddComponent(entity, input);
    }
    return {};
}

std::string SetNetwork(Engine::Registry& registry, Engine::Entity entity,
                       const glz::generic::object_t& fields)
{
    Engine::NetworkComponent network;

    if (registry.HasComponent<Engine::NetworkComponent>(entity)) {
        network = registry.RequireComponent<Engine::NetworkComponent>(entity);
    }
    if (GenericHas(fields, "authority")) {
        network.authority = GenericString(fields, "authority", network.authority);
    }
    if (GenericHas(fields, "syncTransform")) {
        network.syncTransform = GenericBool(fields, "syncTransform", network.syncTransform);
    }
    if (GenericHas(fields, "syncSprite")) {
        network.syncSprite = GenericBool(fields, "syncSprite", network.syncSprite);
    }
    if (GenericHas(fields, "prefabPath")) {
        network.prefabPath = GenericString(fields, "prefabPath", network.prefabPath);
    }
    if (registry.HasComponent<Engine::NetworkComponent>(entity)) {
        registry.RequireComponent<Engine::NetworkComponent>(entity) = network;
    } else {
        registry.AddComponent(entity, network);
    }
    return {};
}

std::string SetRotation(Engine::Registry& registry, Engine::Entity entity,
                        const glz::generic::object_t& fields)
{
    Engine::Rotation rotation = registry.RequireComponent<Engine::Rotation>(entity);

    if (GenericHas(fields, "angle")) {
        rotation.angle = static_cast<float>(GenericNumber(fields, "angle", rotation.angle));
    }
    registry.RequireComponent<Engine::Rotation>(entity) = rotation;
    return {};
}

std::string SetName(Engine::Registry& registry, Engine::Entity entity,
                    const glz::generic::object_t& fields)
{
    std::string value = GenericString(fields, "value");

    if (value.empty()) {
        value = GenericString(fields, "name");
    }
    if (value.empty()) {
        return "name.value is required";
    }
    value = UniqueEntityName(registry, value, entity);
    if (registry.HasComponent<Engine::Name>(entity)) {
        registry.RequireComponent<Engine::Name>(entity).value = value;
    } else {
        registry.AddComponent(entity, Engine::Name::Create(value));
    }
    return {};
}

bool WaveFlag(const glz::generic::object_t& obj, const char* key, bool fallback)
{
    const auto it = obj.find(key);
    std::string text;
    std::size_t index = 0;

    if (it == obj.end()) {
        return fallback;
    }
    if (it->second.is_boolean()) {
        return it->second.get<bool>();
    }
    if (!it->second.is_string()) {
        return fallback;
    }
    text = it->second.get_string();
    for (index = 0; index < text.size(); ++index) {
        text[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(text[index])));
    }
    if (text == "true" || text == "1" || text == "yes") {
        return true;
    }
    if (text == "false" || text == "0" || text == "no") {
        return false;
    }
    return fallback;
}

} // namespace

std::string AllowedComponents()
{
    return "position, rotation, name, sprite, collider, shape, text, opacity, camera, audio, "
           "input, network";
}

std::string ApplyUiAction(Engine::UiAction& action, const glz::generic& value, const char* label)
{
    if (value.is_null() || (!value.is_object() && !value.is_string())) {
        return {};
    }
    if (value.is_string()) {
        action.type = ParseActionType(value.get_string());
        return {};
    }
    const auto& obj = value.get_object();
    if (GenericHas(obj, "type")) {
        action.type = ParseActionType(GenericString(obj, "type"));
    }
    if (GenericHas(obj, "target")) {
        action.target = GenericString(obj, "target");
    }
    if ((action.type == Engine::UiActionType::GoToPage ||
         action.type == Engine::UiActionType::CallMethod ||
         action.type == Engine::UiActionType::LoadScene) &&
        action.target.empty()) {
        return std::string(label) + " needs a target";
    }
    return {};
}

std::string ApplyUiProps(Engine::UiComponent& ui, const glz::generic& props,
                          const std::string& extraLabel)
{
    const Engine::UiTemplateInfo* info = Engine::FindUiTemplate(ui.templateName);
    const Engine::UiField* field = nullptr;
    const Engine::UiField* labelField = nullptr;
    const Engine::UiField* textField = nullptr;
    const Engine::UiField* confirmField = nullptr;
    glz::generic normalized = props;
    std::string caption;
    int index = 0;

    if (info == nullptr) {
        if (!GenericIsObject(normalized) || normalized.get_object().empty()) {
            return {};
        }
        return "entity has no known UI template";
    }
    for (index = 0; index < info->fieldCount; ++index) {
        if (std::string(info->fields[index].key) == "label") {
            labelField = &info->fields[index];
        }
        if (std::string(info->fields[index].key) == "text") {
            textField = &info->fields[index];
        }
        if (std::string(info->fields[index].key) == "confirmLabel") {
            confirmField = &info->fields[index];
        }
    }
    if (!GenericIsObject(normalized)) {
        normalized = glz::generic::object_t{};
    }
    if (!extraLabel.empty() && labelField != nullptr &&
        labelField->type == Engine::UiFieldType::String &&
        !GenericHas(normalized.get_object(), "label")) {
        normalized.get_object()["label"] = extraLabel;
    } else if (!extraLabel.empty() && labelField == nullptr && confirmField != nullptr &&
               confirmField->type == Engine::UiFieldType::String &&
               !GenericHas(normalized.get_object(), "confirmLabel")) {
        normalized.get_object()["confirmLabel"] = extraLabel;
    }
    if (normalized.get_object().empty()) {
        return {};
    }
    if (labelField != nullptr && labelField->type == Engine::UiFieldType::String &&
        textField != nullptr && textField->type == Engine::UiFieldType::Color &&
        GenericHas(normalized.get_object(), "text") &&
        normalized.get_object().at("text").is_string() &&
        !GenericHas(normalized.get_object(), "label")) {
        caption = normalized.get_object().at("text").get_string();
        normalized.get_object().erase("text");
        normalized.get_object()["label"] = caption;
    }
    for (const auto& [key, value] : normalized.get_object()) {
        field = nullptr;
        for (index = 0; index < info->fieldCount; ++index) {
            if (key == info->fields[index].key) {
                field = &info->fields[index];
                break;
            }
        }
        if (field == nullptr) {
            return "unknown UI prop '" + key + "' for template '" + ui.templateName + "'";
        }
        Engine::UiProp& prop = Engine::EnsureUiProp(ui.props, *field);
        if (field->type == Engine::UiFieldType::Bool) {
            if (!value.is_boolean()) {
                return "prop '" + key + "' must be a bool";
            }
            prop.kind = Engine::UiValueKind::Bool;
            prop.boolValue = value.get<bool>();
        } else if (field->type == Engine::UiFieldType::Number) {
            if (!value.is_number()) {
                return "prop '" + key + "' must be a number";
            }
            prop.kind = Engine::UiValueKind::Number;
            prop.numberValue = value.get<double>();
        } else if (field->type == Engine::UiFieldType::Color) {
            prop.kind = Engine::UiValueKind::Color;
            prop.colorSet = true;
            if (value.is_array() && value.get_array().size() >= 3) {
                const auto& items = value.get_array();
                prop.colorValue[0] = static_cast<float>(items[0].get<double>());
                prop.colorValue[1] = static_cast<float>(items[1].get<double>());
                prop.colorValue[2] = static_cast<float>(items[2].get<double>());
                prop.colorValue[3] =
                    items.size() > 3 ? static_cast<float>(items[3].get<double>()) : 1.0f;
            } else {
                return "prop '" + key + "' must be [r,g,b,a]";
            }
        } else if (field->type == Engine::UiFieldType::StringList) {
            if (!value.is_array()) {
                return "prop '" + key + "' must be a string array";
            }
            prop.kind = Engine::UiValueKind::StringList;
            prop.stringList.clear();
            for (const auto& item : value.get_array()) {
                if (!item.is_string()) {
                    return "prop '" + key + "' must be a string array";
                }
                prop.stringList.push_back(item.get_string());
            }
        } else {
            if (!value.is_string()) {
                return "prop '" + key + "' must be a string";
            }
            prop.kind = Engine::UiValueKind::String;
            prop.stringValue = value.get_string();
        }
    }
    return {};
}

std::string SetComponent(Engine::Registry& registry, Engine::Entity entity,
                         const std::string& component, const glz::generic& fields)
{
    glz::generic::object_t empty;
    const glz::generic::object_t* obj = &empty;

    if (GenericIsObject(fields)) {
        obj = &fields.get_object();
    }
    if (component == "position") {
        return SetPosition(registry, entity, *obj);
    }
    if (component == "rotation") {
        return SetRotation(registry, entity, *obj);
    }
    if (component == "name") {
        return SetName(registry, entity, *obj);
    }
    if (component == "sprite") {
        return SetSprite(registry, entity, *obj);
    }
    if (component == "collider") {
        return SetCollider(registry, entity, *obj);
    }
    if (component == "shape") {
        return SetShape(registry, entity, *obj);
    }
    if (component == "text") {
        return SetText(registry, entity, *obj);
    }
    if (component == "opacity") {
        return SetOpacity(registry, entity, *obj);
    }
    if (component == "camera") {
        return SetCamera(registry, entity, *obj);
    }
    if (component == "audio") {
        return SetAudio(registry, entity, *obj);
    }
    if (component == "input") {
        return SetInput(registry, entity, *obj);
    }
    if (component == "network") {
        return SetNetwork(registry, entity, *obj);
    }
    return "unknown component '" + component + "'. allowed: " + AllowedComponents();
}

std::string ValidateLuaFile(const std::filesystem::path& path)
{
    sol::state lua;
    sol::protected_function_result result;
    std::error_code error;

    if (!std::filesystem::is_regular_file(path, error)) {
        return "script not found: " + path.string();
    }
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
    lua.script("log = { info = function() end, error = function() end, debug = function() end }");
    lua.script("Layers = { bit = function() return 0 end, name = function() return '' end }");
    lua.script("UI = { use = function() return { draw = function() end } end, setTheme = "
               "function() end, merge = function(a) return a end }");
    result = lua.script_file(path.string());
    if (!result.valid()) {
        sol::error err = result;
        return err.what();
    }
    if (!result.return_count() || !result[0].is<sol::table>()) {
        return "script must return a table";
    }
    return {};
}

glz::generic AddUiScreen(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    Engine::Entity entity = 0;
    const Engine::UiTemplateInfo* info = nullptr;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (request.templateId.empty()) {
        return MakeResult(false, "template is required");
    }
    info = Engine::FindUiTemplate(request.templateId);
    if (info == nullptr || !Engine::IsUiScreen(*info)) {
        return MakeResult(false, "template is not a UI screen: " + request.templateId);
    }
    entity = session.Registry().CreateEntity();
    if (!request.name.empty()) {
        session.Registry().AddComponent(entity,
                                        Engine::Name::Create(
                                            UniqueEntityName(session.Registry(), request.name)));
    }
    Engine::AttachUiTemplate(session.Registry(), entity, request.templateId);
    if (!session.Registry().HasComponent<Engine::UiComponent>(entity)) {
        return MakeResult(false, "failed to attach UI template");
    }
    if (request.startPage.value_or(Engine::ListUiPages(session.Registry()).size() == 1)) {
        Engine::SetUiStartPage(session.Registry(), entity);
    }
    error = ApplyUiProps(session.Registry().RequireComponent<Engine::UiComponent>(entity),
                         request.props, request.label);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    auto& obj = result.get_object();
    obj["id"] = static_cast<double>(entity);
    obj["name"] = EntityDisplayName(session.Registry(), entity);
    obj["page"] = session.Registry().RequireComponent<Engine::UiComponent>(entity).page;
    obj["scenePath"] = session.ScenePath().string();
    return result;
}

glz::generic AddUiWidget(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> screen;
    Engine::Entity child = 0;
    const Engine::UiTemplateInfo* info = nullptr;
    std::string saveError;
    Engine::UiComponent* ui = nullptr;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (request.templateId.empty()) {
        return MakeResult(false, "template is required");
    }
    info = Engine::FindUiTemplate(request.templateId);
    if (info == nullptr || !Engine::IsUiWidget(*info)) {
        return MakeResult(false, "template is not a UI widget: " + request.templateId);
    }
    screen = FindScreen(session.Registry(), request.screen);
    if (!screen.has_value()) {
        return MakeResult(false, "screen not found: " + request.screen);
    }
    child = Engine::AddUiChild(session.Registry(), screen.value(), request.templateId);
    ui = &session.Registry().RequireComponent<Engine::UiComponent>(child);
    error = ApplyUiProps(*ui, request.props, request.label);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    error = ApplyUiAction(ui->onClick, request.onClick, "onClick");
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    error = ApplyUiAction(ui->onCancel, request.onCancel, "onCancel");
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (!request.name.empty()) {
        session.Registry().RequireComponent<Engine::Name>(child).value =
            UniqueEntityName(session.Registry(), request.name, child);
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    auto& obj = result.get_object();
    obj["id"] = static_cast<double>(child);
    obj["name"] = EntityDisplayName(session.Registry(), child);
    obj["page"] = Engine::UiPageOf(session.Registry(), child);
    obj["scenePath"] = session.ScenePath().string();
    return result;
}

glz::generic SetUiProps(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> entity;
    Engine::UiComponent* ui = nullptr;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = FindEntity(session.Registry(), request.entity);
    if (!entity.has_value()) {
        return MakeResult(false, "entity not found: " + request.entity);
    }
    if (!session.Registry().HasComponent<Engine::UiComponent>(entity.value())) {
        return MakeResult(false, "entity has no UI component");
    }
    ui = &session.Registry().RequireComponent<Engine::UiComponent>(entity.value());
    error = ApplyUiProps(*ui, request.props, request.label);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    error = ApplyUiAction(ui->onClick, request.onClick, "onClick");
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    error = ApplyUiAction(ui->onCancel, request.onCancel, "onCancel");
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    return MakeResult(true, "");
}

glz::generic PlacePrefab(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    Engine::Entity root = 0;
    std::string relative;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (request.prefabPath.empty()) {
        return MakeResult(false, "prefabPath is required");
    }
    root = Serialization::SpawnPrefabRuntime(session.Registry(), request.prefabPath);
    if (root == static_cast<Engine::Entity>(-1)) {
        return MakeResult(false, "failed to spawn prefab: " + request.prefabPath);
    }
    if (request.x.has_value() || request.y.has_value()) {
        Engine::Position& position = session.Registry().RequireComponent<Engine::Position>(root);
        if (request.x.has_value()) {
            position.x = static_cast<float>(request.x.value());
        }
        if (request.y.has_value()) {
            position.y = static_cast<float>(request.y.value());
        }
        position.prevX = position.x;
        position.prevY = position.y;
        position.moved = false;
    }
    relative = Engine::ToProjectRelativePath(session.Registry(), request.prefabPath);
    session.Registry().RegisterComponent<Engine::PrefabInstance>();
    if (session.Registry().HasComponent<Engine::PrefabInstance>(root)) {
        session.Registry().RequireComponent<Engine::PrefabInstance>(root).prefabPath = relative;
    } else {
        session.Registry().AddComponent(root, Engine::PrefabInstance::Create(relative));
    }
    if (!request.name.empty()) {
        if (session.Registry().HasComponent<Engine::Name>(root)) {
            session.Registry().RequireComponent<Engine::Name>(root).value =
                UniqueEntityName(session.Registry(), request.name, root);
        } else {
            session.Registry().AddComponent(
                root, Engine::Name::Create(UniqueEntityName(session.Registry(), request.name, root)));
        }
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    result.get_object()["id"] = static_cast<double>(root);
    result.get_object()["name"] = EntityDisplayName(session.Registry(), root);
    result.get_object()["prefabPath"] = relative;
    return result;
}

glz::generic CreateEntityCmd(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    Engine::Entity entity = 0;
    std::string name;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = session.Registry().CreateEntity();
    name = UniqueEntityName(session.Registry(), request.name.empty() ? "Entity" : request.name);
    session.Registry().AddComponent(entity, Engine::Name::Create(name));
    if (request.x.has_value() || request.y.has_value() || request.z.has_value()) {
        Engine::Position& position = session.Registry().RequireComponent<Engine::Position>(entity);
        if (request.x.has_value()) {
            position.x = static_cast<float>(request.x.value());
        }
        if (request.y.has_value()) {
            position.y = static_cast<float>(request.y.value());
        }
        if (request.z.has_value()) {
            position.z = request.z.value();
        }
        position.prevX = position.x;
        position.prevY = position.y;
        position.prevZ = position.z;
        position.moved = false;
    }
    if (GenericIsArray(request.components)) {
        for (const auto& item : request.components.get_array()) {
            if (!item.is_object()) {
                return MakeResult(false, "components entries must be objects");
            }
            const auto& obj = item.get_object();
            error = SetComponent(session.Registry(), entity, GenericString(obj, "component"),
                                 GenericHas(obj, "fields") ? obj.at("fields") : glz::generic{});
            if (!error.empty()) {
                return MakeResult(false, error);
            }
        }
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    result.get_object()["id"] = static_cast<double>(entity);
    result.get_object()["name"] = name;
    return result;
}

glz::generic SetComponentCmd(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> entity;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = FindEntity(session.Registry(), request.entity);
    if (!entity.has_value()) {
        return MakeResult(false, "entity not found: " + request.entity);
    }
    if (request.component.empty()) {
        return MakeResult(false, "component is required. allowed: " + AllowedComponents());
    }
    error = SetComponent(session.Registry(), entity.value(), request.component, request.fields);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    result.get_object()["id"] = static_cast<double>(entity.value());
    result.get_object()["name"] = EntityDisplayName(session.Registry(), entity.value());
    result.get_object()["component"] = request.component;
    return result;
}

glz::generic AttachScript(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> entity;
    std::string relative = request.scriptPath;
    std::filesystem::path full;
    std::error_code fsError;
    bool created = false;
    std::string saveError;
    bool autoReload = request.autoReload.value_or(true);

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = FindEntity(session.Registry(), request.entity);
    if (!entity.has_value()) {
        return MakeResult(false, "entity not found: " + request.entity);
    }
    if (relative.empty()) {
        relative = "scripts/" +
                   SanitizeFileStem(EntityDisplayName(session.Registry(), entity.value())) + ".lua";
    }
    full = session.ProjectFolder() / relative;
    std::filesystem::create_directories(full.parent_path(), fsError);
    if (!std::filesystem::exists(full)) {
        error = WriteScriptFile(full, EntityDisplayName(session.Registry(), entity.value()),
                                request.hooks);
        if (!error.empty()) {
            return MakeResult(false, error);
        }
        created = true;
    } else if (GenericIsObject(request.hooks) && !request.hooks.get_object().empty()) {
        return MakeResult(false, "script already exists; omit hooks to attach it, or use a new "
                                 "scriptPath");
    }
    error = ValidateLuaFile(full);
    if (!error.empty()) {
        if (created) {
            std::filesystem::remove(full, fsError);
        }
        return MakeResult(false, "validate_script failed: " + error);
    }
    if (created) {
        LiveBinding* live = CurrentLiveBinding();
        ScriptFileChange change;

        if (live != nullptr && live->files != nullptr) {
            change.path = full.string();
            change.existed = false;
            live->files->push_back(change);
        }
    }
    if (session.Registry().HasComponent<Engine::ScriptComponent>(entity.value())) {
        Engine::ScriptComponent& script =
            session.Registry().RequireComponent<Engine::ScriptComponent>(entity.value());
        script.scriptPath = relative;
        script.autoReload = autoReload;
        script.loaded = false;
        script.scriptTable = sol::lua_nil;
    } else {
        session.Registry().AddComponent(entity.value(),
                                        Engine::ScriptComponent(relative, autoReload));
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    result.get_object()["scriptPath"] = relative;
    result.get_object()["created"] = created;
    return result;
}

glz::generic AddWave(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    Engine::WaveDefinition wave;
    const glz::generic::object_t* obj = nullptr;
    std::string saveError;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (!GenericIsObject(request.wave)) {
        return MakeResult(false, "wave object is required");
    }
    obj = &request.wave.get_object();
    wave.name = GenericString(*obj, "name", "Wave " + std::to_string(session.Config().m_Waves.size() + 1));
    wave.triggerX = static_cast<float>(GenericNumber(*obj, "triggerX", 0.0));
    wave.prefabPath = GenericString(*obj, "prefabPath");
    if (wave.prefabPath.empty()) {
        return MakeResult(false, "wave.prefabPath is required");
    }
    wave.formation = ParseFormation(GenericString(*obj, "formation", "line"));
    wave.count = static_cast<unsigned int>(GenericNumber(*obj, "count", 1));
    wave.spacing = static_cast<float>(GenericNumber(*obj, "spacing", 100.0));
    wave.baseY = static_cast<float>(GenericNumber(*obj, "baseY", 0.0));
    wave.spawnOffsetX = static_cast<float>(GenericNumber(*obj, "spawnOffsetX", 900.0));
    wave.attachParent = WaveFlag(*obj, "attachParent", false);
    wave.groupScriptPath = GenericString(*obj, "groupScriptPath");
    session.Config().m_Waves.push_back(wave);
    if (!session.Registry().HasResource<Engine::WaveSchedule>()) {
        session.Registry().AddResource<Engine::WaveSchedule>();
    }
    session.Registry().GetResource<Engine::WaveSchedule>().waves = session.Config().m_Waves;
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    glz::generic result = MakeResult(true, "");
    result.get_object()["name"] = wave.name;
    result.get_object()["index"] = static_cast<double>(session.Config().m_Waves.size() - 1);
    return result;
}

int FindWaveIndex(const std::vector<Engine::WaveDefinition>& waves, const glz::generic::object_t& obj)
{
    int index = 0;
    std::string name;
    int found = -1;
    std::size_t cursor = 0;

    if (GenericHas(obj, "index")) {
        index = static_cast<int>(GenericNumber(obj, "index", -1.0));
        if (index < 0 || static_cast<std::size_t>(index) >= waves.size()) {
            return -1;
        }
        return index;
    }
    name = GenericString(obj, "name");
    if (name.empty()) {
        return -1;
    }
    for (cursor = 0; cursor < waves.size(); ++cursor) {
        if (waves[cursor].name != name) {
            continue;
        }
        if (found >= 0) {
            return -2;
        }
        found = static_cast<int>(cursor);
    }
    return found;
}

void ApplyWaveFields(Engine::WaveDefinition& wave, const glz::generic::object_t& obj, bool create)
{
    if (create || GenericHas(obj, "name")) {
        if (GenericHas(obj, "name") || create) {
            std::string name = GenericString(obj, "name", wave.name);
            if (!name.empty()) {
                wave.name = name;
            }
        }
    }
    if (GenericHas(obj, "triggerX")) {
        wave.triggerX = static_cast<float>(GenericNumber(obj, "triggerX", wave.triggerX));
    }
    if (GenericHas(obj, "prefabPath")) {
        wave.prefabPath = GenericString(obj, "prefabPath", wave.prefabPath);
    }
    if (GenericHas(obj, "formation")) {
        wave.formation = ParseFormation(GenericString(obj, "formation", "line"));
    }
    if (GenericHas(obj, "count")) {
        wave.count = static_cast<unsigned int>(GenericNumber(obj, "count", wave.count));
    }
    if (GenericHas(obj, "spacing")) {
        wave.spacing = static_cast<float>(GenericNumber(obj, "spacing", wave.spacing));
    }
    if (GenericHas(obj, "baseY")) {
        wave.baseY = static_cast<float>(GenericNumber(obj, "baseY", wave.baseY));
    }
    if (GenericHas(obj, "spawnOffsetX")) {
        wave.spawnOffsetX = static_cast<float>(GenericNumber(obj, "spawnOffsetX", wave.spawnOffsetX));
    }
    if (GenericHas(obj, "attachParent")) {
        wave.attachParent = WaveFlag(obj, "attachParent", wave.attachParent);
    }
    if (GenericHas(obj, "groupScriptPath")) {
        wave.groupScriptPath = GenericString(obj, "groupScriptPath", wave.groupScriptPath);
    }
}

void SyncWaves(Session& session)
{
    if (!session.Registry().HasResource<Engine::WaveSchedule>()) {
        session.Registry().AddResource<Engine::WaveSchedule>();
    }
    session.Registry().GetResource<Engine::WaveSchedule>().waves = session.Config().m_Waves;
}

struct PrefabFile {
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> entities;
};

void CollectPrefab(Engine::Registry& registry, Engine::Entity entity,
                   Serialization::GlazeSerializer& serializer,
                   std::vector<Serialization::GlazeSerializer::EntitySnapshot>& out,
                   std::vector<Engine::Entity>& seen, int depth)
{
    std::size_t index = 0;
    std::vector<Engine::Entity> children;

    if (depth > 64 || !registry.IsEntityAlive(entity)) {
        return;
    }
    for (index = 0; index < seen.size(); ++index) {
        if (seen[index] == entity) {
            return;
        }
    }
    seen.push_back(entity);
    out.push_back(serializer.CaptureEntity(registry, entity));
    children = Engine::HierarchySystem::FindDirectChildren(registry, entity);
    for (Engine::Entity child : children) {
        CollectPrefab(registry, child, serializer, out, seen, depth + 1);
    }
}

bool WritePrefabFile(const std::filesystem::path& path,
                     const std::vector<Serialization::GlazeSerializer::EntitySnapshot>& snapshots)
{
    PrefabFile file;
    Serialization::GlazeSerializer serializer;

    file.entities = snapshots;
    return serializer.SerializeToFile(path.string(), file);
}

std::optional<Engine::Entity> FindNamedDescendant(Engine::Registry& registry, Engine::Entity root,
                                                  const std::string& name, int depth)
{
    std::vector<Engine::Entity> children;
    std::optional<Engine::Entity> found;

    if (depth > 64 || !registry.IsEntityAlive(root)) {
        return std::nullopt;
    }
    if (EntityDisplayName(registry, root) == name) {
        return root;
    }
    children = Engine::HierarchySystem::FindDirectChildren(registry, root);
    for (Engine::Entity child : children) {
        found = FindNamedDescendant(registry, child, name, depth + 1);
        if (found.has_value()) {
            return found;
        }
    }
    return std::nullopt;
}

bool SamePrefabPath(const std::string& left, const std::string& right)
{
    std::filesystem::path a = left;
    std::filesystem::path b = right;

    return a.lexically_normal().generic_string() == b.lexically_normal().generic_string();
}

void RememberFile(const std::filesystem::path& path)
{
    LiveBinding* live = CurrentLiveBinding();
    ScriptFileChange change;
    std::ifstream in;
    std::error_code error;

    if (live == nullptr || live->files == nullptr) {
        return;
    }
    change.path = path.string();
    change.existed = std::filesystem::is_regular_file(path, error) && !error;
    if (change.existed) {
        in.open(path, std::ios::binary);
        change.previous.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    live->files->push_back(std::move(change));
}

glz::generic SetWave(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    const glz::generic::object_t* obj = nullptr;
    int index = 0;
    Engine::WaveDefinition wave;
    std::string saveError;
    glz::generic result;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (!GenericIsObject(request.wave)) {
        return MakeResult(false, "wave object is required");
    }
    obj = &request.wave.get_object();
    if (!GenericHas(*obj, "index")) {
        return MakeResult(false, "wave.index is required");
    }
    index = FindWaveIndex(session.Config().m_Waves, *obj);
    if (index < 0) {
        return MakeResult(false, "wave not found");
    }
    wave = session.Config().m_Waves[static_cast<std::size_t>(index)];
    ApplyWaveFields(wave, *obj, false);
    if (wave.prefabPath.empty()) {
        return MakeResult(false, "wave.prefabPath is required");
    }
    session.Config().m_Waves[static_cast<std::size_t>(index)] = wave;
    SyncWaves(session);
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    result = MakeResult(true, "");
    result.get_object()["index"] = static_cast<double>(index);
    result.get_object()["name"] =
        session.Config().m_Waves[static_cast<std::size_t>(index)].name;
    return result;
}

glz::generic RemoveWave(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    const glz::generic::object_t* obj = nullptr;
    int index = 0;
    std::string removed;
    std::string saveError;
    glz::generic result;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (!GenericIsObject(request.wave)) {
        return MakeResult(false, "wave object is required");
    }
    obj = &request.wave.get_object();
    index = FindWaveIndex(session.Config().m_Waves, *obj);
    if (index == -2) {
        return MakeResult(false, "more than one wave has that name; pass wave.index");
    }
    if (index < 0) {
        return MakeResult(false, "wave not found");
    }
    removed = session.Config().m_Waves[static_cast<std::size_t>(index)].name;
    session.Config().m_Waves.erase(session.Config().m_Waves.begin() + index);
    SyncWaves(session);
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    result = MakeResult(true, "");
    result.get_object()["index"] = static_cast<double>(index);
    result.get_object()["name"] = removed;
    return result;
}

glz::generic CreatePrefab(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    std::optional<Engine::Entity> entity;
    std::filesystem::path file;
    std::filesystem::path folder;
    std::error_code fsError;
    Serialization::GlazeSerializer serializer;
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> snapshots;
    std::vector<Engine::Entity> seen;
    std::string relative;
    std::string previousPrefab;
    bool hadPrefab = false;
    std::string saveError;
    glz::generic result;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    entity = FindEntity(session.Registry(), request.entity);
    if (!entity.has_value()) {
        return MakeResult(false, "entity not found: " + request.entity);
    }
    folder = session.ProjectFolder() / "Prefabs";
    std::filesystem::create_directories(folder, fsError);
    if (request.prefabPath.empty()) {
        file = folder / (SanitizeFileStem(request.name.empty()
                                              ? EntityDisplayName(session.Registry(), entity.value())
                                              : request.name) +
                         ".prefab");
    } else {
        file = request.prefabPath;
        if (!file.is_absolute()) {
            file = session.ProjectFolder() / file;
        }
        if (file.extension().empty()) {
            file += ".prefab";
        }
    }
    if (std::filesystem::exists(file)) {
        return MakeResult(false, "prefab already exists: " + file.string());
    }
    CollectPrefab(session.Registry(), entity.value(), serializer, snapshots, seen, 0);
    if (snapshots.empty()) {
        return MakeResult(false, "failed to capture prefab");
    }
    snapshots.front().parent.reset();
    snapshots.front().prefab.reset();
    if (snapshots.front().position.has_value()) {
        snapshots.front().position->relative = false;
    }
    RememberFile(file);
    if (!WritePrefabFile(file, snapshots)) {
        return MakeResult(false, "failed to write prefab: " + file.string());
    }
    relative = Engine::ToProjectRelativePath(session.Registry(), file.string());
    hadPrefab = session.Registry().HasComponent<Engine::PrefabInstance>(entity.value());
    if (hadPrefab) {
        previousPrefab =
            session.Registry().RequireComponent<Engine::PrefabInstance>(entity.value()).prefabPath;
        session.Registry().RequireComponent<Engine::PrefabInstance>(entity.value()).prefabPath =
            relative;
    } else {
        session.Registry().AddComponent(entity.value(), Engine::PrefabInstance::Create(relative));
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        if (hadPrefab) {
            session.Registry().RequireComponent<Engine::PrefabInstance>(entity.value()).prefabPath =
                previousPrefab;
        } else if (session.Registry().HasComponent<Engine::PrefabInstance>(entity.value())) {
            session.Registry().RemoveComponent<Engine::PrefabInstance>(entity.value());
        }
        return MakeResult(false, saveError);
    }
    result = MakeResult(true, "");
    result.get_object()["prefabPath"] = relative;
    result.get_object()["entity"] = EntityDisplayName(session.Registry(), entity.value());
    return result;
}

glz::generic SetPrefabComponent(const AuthorRequest& request)
{
    Session session;
    std::string error = OpenSession(request, session);
    Engine::Registry prefabRegistry;
    Engine::Entity root = static_cast<Engine::Entity>(-1);
    std::optional<Engine::Entity> edited;
    std::string editedName;
    bool editedRoot = false;
    Serialization::GlazeSerializer serializer;
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> snapshots;
    std::vector<Engine::Entity> seen;
    std::filesystem::path file;
    std::string relative;
    std::string saveError;
    std::size_t index = 0;
    glz::generic result;

    if (!error.empty()) {
        return MakeResult(false, error);
    }
    if (request.prefabPath.empty()) {
        return MakeResult(false, "prefabPath is required");
    }
    if (request.component.empty()) {
        return MakeResult(false, "component is required");
    }
    RegisterAuthorComponents(prefabRegistry);
    Serialization::RegisterRuntimePrefabSpawner(prefabRegistry);
    if (!prefabRegistry.HasResource<Engine::ProjectContext>()) {
        prefabRegistry.AddResource<Engine::ProjectContext>(session.ProjectFolder().string());
    }
    root = Serialization::SpawnPrefabRuntime(prefabRegistry, request.prefabPath);
    if (root == static_cast<Engine::Entity>(-1)) {
        return MakeResult(false, "failed to read prefab: " + request.prefabPath);
    }
    if (request.entity.empty()) {
        edited = root;
    } else {
        edited = FindEntity(prefabRegistry, request.entity);
    }
    if (!edited.has_value()) {
        return MakeResult(false, "entity not found in prefab: " + request.entity);
    }
    error = SetComponent(prefabRegistry, edited.value(), request.component, request.fields);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    editedName = EntityDisplayName(prefabRegistry, edited.value());
    editedRoot = edited.value() == root;
    CollectPrefab(prefabRegistry, root, serializer, snapshots, seen, 0);
    if (snapshots.empty()) {
        return MakeResult(false, "failed to capture prefab");
    }
    snapshots.front().parent.reset();
    snapshots.front().prefab.reset();
    file = Engine::ResolveAssetPath(prefabRegistry, request.prefabPath);
    RememberFile(file);
    if (!WritePrefabFile(file, snapshots)) {
        return MakeResult(false, "failed to write prefab: " + file.string());
    }
    relative = Engine::ToProjectRelativePath(session.Registry(), file.string());
    if (session.Registry().IsComponentRegistered<Engine::PrefabInstance>()) {
        for (index = 1; index < session.Registry().GetEntityCount(); ++index) {
            Engine::Entity instance = Engine::Entity{index};
            std::optional<Engine::Entity> target;
            if (!session.Registry().IsEntityAlive(instance) ||
                !session.Registry().HasComponent<Engine::PrefabInstance>(instance)) {
                continue;
            }
            if (!SamePrefabPath(
                    session.Registry().RequireComponent<Engine::PrefabInstance>(instance).prefabPath,
                    relative)) {
                continue;
            }
            session.Registry().RequireComponent<Engine::PrefabInstance>(instance).prefabPath =
                relative;
            if (editedRoot) {
                target = instance;
            } else {
                target = FindNamedDescendant(session.Registry(), instance, editedName, 0);
            }
            if (!target.has_value()) {
                continue;
            }
            error = SetComponent(session.Registry(), target.value(), request.component, request.fields);
            if (!error.empty()) {
                return MakeResult(false, error);
            }
        }
    }
    saveError = session.Save();
    if (!saveError.empty()) {
        return MakeResult(false, saveError);
    }
    result = MakeResult(true, "");
    result.get_object()["prefabPath"] = relative;
    result.get_object()["entity"] = editedName;
    result.get_object()["component"] = request.component;
    return result;
}

glz::generic ValidateScript(const AuthorRequest& request)
{
    std::filesystem::path project = ResolveProjectFolder(request.projectFolder);
    std::filesystem::path path;
    std::string error;
    glz::generic result;

    if (request.scriptPath.empty()) {
        return MakeResult(false, "scriptPath is required");
    }
    path = request.scriptPath;
    if (!path.is_absolute() && !project.empty()) {
        path = project / path;
    }
    error = ValidateLuaFile(path);
    if (!error.empty()) {
        return MakeResult(false, error);
    }
    result = MakeResult(true, "");
    result.get_object()["scriptPath"] = path.string();
    return result;
}

} // namespace Author
