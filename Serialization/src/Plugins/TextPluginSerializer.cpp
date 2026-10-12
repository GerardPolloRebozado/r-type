#include "Plugins/TextPluginSerializer.hpp"
#include "Components/Text.hpp"
#include "Plugins/TextPlugin.hpp"

namespace Serialization {

void TextPluginSerializer::Serialize(Engine::Entity entity, Engine::Registry& registry,
                                     nlohmann::json& outEntityJson) {
    if (registry.HasComponent<Engine::Text>(entity)) {
        const auto& text = registry.GetComponent<Engine::Text>(entity);
        outEntityJson["TextComponent"] = {{"text", text.text}, {"fontSize", text.fontsize}};
    }
}

void TextPluginSerializer::Deserialize(Engine::Entity entity, Engine::Registry& registry,
                                       const nlohmann::json& inEntityJson) {
    if (inEntityJson.contains("TextComponent")) {
        const auto& textJson = inEntityJson["TextComponent"];
        std::string text = textJson.value("text", "");
        float fontSize = textJson.value("fontSize", 12.0f);
        Engine::TextPlugin::AttachToEntity(registry, entity, text, fontSize);
    }
}

} // namespace Serialization
