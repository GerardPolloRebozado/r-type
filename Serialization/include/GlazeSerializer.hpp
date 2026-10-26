#ifndef SERIALIZATION_GLAZESERIALIZER_HPP_
#define SERIALIZATION_GLAZESERIALIZER_HPP_

#include "Components/CameraComponent.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/SignalConnections.hpp"
#include "ECS/Registry.hpp"
#include "GlazeReflect.hpp"
#include "Reflection/ComponentReflect.hpp"
#include "Scripting/ScriptComponent.hpp"
#include <cstddef>
#include <cstring>
#include <fstream>
#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <vector>

namespace Serialization {

enum class SerializationFormat { JSON, Binary };

class GlazeSerializer {
  public:
    explicit GlazeSerializer(SerializationFormat format = SerializationFormat::JSON);

    template <typename T> std::string SerializeToString(const T& value);

    template <typename T> std::vector<std::byte> SerializeToBinary(const T& value);

    template <typename T> bool DeserializeFromString(const std::string& data, T& value);

    template <typename T> bool DeserializeFromBinary(const std::vector<std::byte>& data, T& value);

    template <typename T> bool SerializeToFile(const std::string& filepath, const T& value);

    template <typename T> bool DeserializeFromFile(const std::string& filepath, T& value);

    // Same as above, but captures a human-readable parse error (empty when
    // the file itself could not be opened).
    template <typename T>
    bool DeserializeFromFile(const std::string& filepath, T& value, std::string* error);

    SerializationFormat GetFormat() const;
    void SetFormat(SerializationFormat format);

    struct EntitySnapshot {
        Engine::Entity entityId;
        std::optional<Engine::Name> name;
        std::optional<Engine::Position> position;
        std::optional<Engine::Rotation> rotation;
        std::optional<Engine::SpriteComponent> sprite;
        std::optional<Engine::ShapeComponent> shape;
        std::optional<Engine::Text> text;
        std::optional<Engine::ColliderComponent> collider;
        std::optional<Engine::Opacity> opacity;
        std::optional<Engine::SignalConnections> connections;
        std::optional<Engine::Parent> parent;
        std::optional<Engine::Input> input;
        std::optional<Engine::AudioSource> audioSource;
        std::optional<Engine::CameraComponent> camera;
        std::optional<Engine::ScriptComponent> script;
        std::optional<Engine::NetworkComponent> network;
        // Absent on old scenes. Present only on a subtree root tied to a .prefab.
        std::optional<Engine::PrefabInstance> prefab;
    };

    struct SceneSnapshot {
        std::string sceneName = "Untitled";
        std::vector<EntitySnapshot> entities;
    };

    SceneSnapshot CaptureScene(Engine::Registry& registry);
    void RestoreScene(Engine::Registry& registry, const SceneSnapshot& snapshot);

    EntitySnapshot CaptureEntity(Engine::Registry& registry, Engine::Entity entity);
    // New ids. The first snapshot becomes the returned root. Empty input returns (Entity)-1.
    Engine::Entity SpawnEntities(Engine::Registry& registry,
                                 const std::vector<EntitySnapshot>& entities);

  private:
    // Files saved before ShapeComponent had explicit key names may use
    // "color)" instead of "color": automatic reflection read the name from
    // compiler-generated text, which differs between compilers. Rewrites the
    // old key so those files load on every OS.
    static void NormalizeLegacyKeys(std::string& json);

    SerializationFormat m_Format;
};

inline void GlazeSerializer::NormalizeLegacyKeys(std::string& json) {
    static const std::string from = "\"color)\":";
    static const std::string to = "\"color\":";
    std::size_t pos = 0;
    while ((pos = json.find(from, pos)) != std::string::npos) {
        json.replace(pos, from.size(), to);
        pos += to.size();
    }
}

template <typename T> std::string GlazeSerializer::SerializeToString(const T& value) {
    if (m_Format == SerializationFormat::Binary) {
        auto binary = SerializeToBinary(value);
        return std::string(reinterpret_cast<const char*>(binary.data()), binary.size());
    }
    std::string output;
    if (glz::write_json(value, output)) {
        return "";
    }
    return output;
}

template <typename T> std::vector<std::byte> GlazeSerializer::SerializeToBinary(const T& value) {
    std::vector<std::byte> output;
    if (glz::write_beve(value, output)) {
        return {};
    }
    return output;
}

template <typename T>
bool GlazeSerializer::DeserializeFromString(const std::string& data, T& value) {
    if (m_Format == SerializationFormat::Binary) {
        std::vector<std::byte> binary(data.size());
        std::memcpy(binary.data(), data.data(), data.size());
        return DeserializeFromBinary(binary, value);
    }
    std::string json = data;
    NormalizeLegacyKeys(json);
    return !glz::read_json(value, json);
}

template <typename T>
bool GlazeSerializer::DeserializeFromBinary(const std::vector<std::byte>& data, T& value) {
    return !glz::read_beve(value, data);
}

template <typename T>
bool GlazeSerializer::SerializeToFile(const std::string& filepath, const T& value) {
    if (m_Format == SerializationFormat::Binary) {
        auto binary = SerializeToBinary(value);
        if (binary.empty()) {
            return false;
        }
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        file.write(reinterpret_cast<const char*>(binary.data()), binary.size());
        return file.good();
    }
    std::string output = SerializeToString(value);
    if (output.empty()) {
        return false;
    }
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    file << output;
    return file.good();
}

template <typename T>
bool GlazeSerializer::DeserializeFromFile(const std::string& filepath, T& value) {
    return DeserializeFromFile(filepath, value, nullptr);
}

template <typename T>
bool GlazeSerializer::DeserializeFromFile(const std::string& filepath, T& value,
                                          std::string* error) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (m_Format == SerializationFormat::Binary) {
        std::vector<std::byte> binary(size);
        file.read(reinterpret_cast<char*>(binary.data()), size);
        return DeserializeFromBinary(binary, value);
    }

    std::string content(size, '\0');
    file.read(content.data(), size);
    NormalizeLegacyKeys(content);
    const glz::error_ctx err = glz::read_json(value, content);
    if (err) {
        if (error != nullptr) {
            *error = glz::format_error(err, content);
        }
        return false;
    }
    return true;
}

} // namespace Serialization

#endif // SERIALIZATION_GLAZESERIALIZER_HPP_