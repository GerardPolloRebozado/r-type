#ifndef AUTHOR_PROTOCOL_HPP_
#define AUTHOR_PROTOCOL_HPP_

#include <glaze/glaze.hpp>
#include <optional>
#include <string>

namespace Author {

inline constexpr int kApiVersion = 1;

/**
 * @brief One JSON command sent on stdin to rtype-author.
 *
 * Unknown keys are ignored so a newer adapter can add fields.
 */
struct AuthorRequest {
    int apiVersion = kApiVersion;
    std::string command;
    std::string projectFolder;
    std::string scenePath;
    std::string engineRoot;
    std::string entity;
    std::string name;
    std::string templateId;
    std::string screen;
    std::string component;
    std::string prefabPath;
    std::string scriptPath;
    std::string topic;
    std::string page;
    std::optional<double> x;
    std::optional<double> y;
    std::optional<int> z;
    std::optional<bool> startPage;
    std::optional<bool> autoReload;
    glz::generic props{};
    glz::generic fields{};
    glz::generic hooks{};
    glz::generic onClick{};
    glz::generic onCancel{};
    glz::generic wave{};
    glz::generic components{};
};

inline bool GenericIsObject(const glz::generic& value)
{
    return value.is_object();
}

inline bool GenericIsArray(const glz::generic& value)
{
    return value.is_array();
}

inline std::string GenericString(const glz::generic::object_t& obj, const char* key,
                                 const std::string& fallback = {})
{
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_string()) {
        return fallback;
    }
    return it->second.get_string();
}

inline bool GenericBool(const glz::generic::object_t& obj, const char* key, bool fallback = false)
{
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_boolean()) {
        return fallback;
    }
    return it->second.get<bool>();
}

inline double GenericNumber(const glz::generic::object_t& obj, const char* key, double fallback = 0.0)
{
    const auto it = obj.find(key);
    if (it == obj.end() || !it->second.is_number()) {
        return fallback;
    }
    return it->second.get<double>();
}

inline bool GenericHas(const glz::generic::object_t& obj, const char* key)
{
    return obj.find(key) != obj.end();
}

} // namespace Author

template <> struct glz::meta<Author::AuthorRequest> {
    static constexpr auto value = glz::object(
        "apiVersion", &Author::AuthorRequest::apiVersion, "command", &Author::AuthorRequest::command,
        "projectFolder", &Author::AuthorRequest::projectFolder, "scenePath",
        &Author::AuthorRequest::scenePath, "engineRoot", &Author::AuthorRequest::engineRoot,
        "entity", &Author::AuthorRequest::entity, "name", &Author::AuthorRequest::name, "template",
        &Author::AuthorRequest::templateId, "screen", &Author::AuthorRequest::screen, "component",
        &Author::AuthorRequest::component, "prefabPath", &Author::AuthorRequest::prefabPath,
        "scriptPath", &Author::AuthorRequest::scriptPath, "topic", &Author::AuthorRequest::topic,
        "page", &Author::AuthorRequest::page, "x", &Author::AuthorRequest::x, "y",
        &Author::AuthorRequest::y, "z", &Author::AuthorRequest::z, "startPage",
        &Author::AuthorRequest::startPage, "autoReload", &Author::AuthorRequest::autoReload,
        "props", &Author::AuthorRequest::props, "fields", &Author::AuthorRequest::fields, "hooks",
        &Author::AuthorRequest::hooks, "onClick", &Author::AuthorRequest::onClick, "onCancel",
        &Author::AuthorRequest::onCancel, "wave", &Author::AuthorRequest::wave, "components",
        &Author::AuthorRequest::components);
};

#endif // AUTHOR_PROTOCOL_HPP_
