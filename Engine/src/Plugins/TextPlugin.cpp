#include "Plugins/TextPlugin.hpp"

namespace Engine {
void TextPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Text>();
    registry.RegisterSystem<TextSystem>();
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt) {
    registry.AddComponent(entity, Text{txt, 10.0f, WHITE});
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize) {
    registry.AddComponent(entity, Text{txt, fontSize, WHITE});
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize, const Position position) {
    registry.AddComponent(entity, Text{txt, fontSize, WHITE});
    registry.AddComponent(entity, position);
}
}
