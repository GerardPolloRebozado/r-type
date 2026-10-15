#ifndef SERIALIZATION_GLAZESERIALIZER_HPP_
#define SERIALIZATION_GLAZESERIALIZER_HPP_

#include "Components/CameraComponent.hpp"
#include "Components/Opacity.hpp"
#include "ECS/Registry.hpp"
#include "GlazeReflect.hpp"
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
        std::optional<Engine::Parent> parent;
        std::optional<Engine::Input> input;
        std::optional<Engine::AudioSource> audioSource;
        std::optional<Engine::CameraComponent> camera;
        std::optional<Engine::ScriptComponent> script;
    };

    struct SceneSnapshot {
        std::string sceneName = "Untitled";
        std::vector<EntitySnapshot> entities;
    };

    SceneSnapshot CaptureScene(Engine::Registry& registry);
    void RestoreScene(Engine::Registry& registry, const SceneSnapshot& snapshot,
                      std::unordered_map<Engine::Entity, Engine::Entity>* outRemap = nullptr);

  private:
    SerializationFormat m_Format;
};

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
    return !glz::read_json(value, data);
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
    return DeserializeFromString(content, value);
}

} // namespace Serialization

#endif // SERIALIZATION_GLAZESERIALIZER_HPP_