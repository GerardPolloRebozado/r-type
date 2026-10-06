#include "Plugins/TextPlugin.hpp"

namespace Engine {
void TextPlugin::Init(Registry& registry) {
    registry.RegisterComponent<Text>();
    registry.RegisterSystem<TextSystem>();
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt) {
    registry.AddComponent(entity, Text{txt, 10.0f, WHITE});
    registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                                const float fontSize) {
    registry.AddComponent(entity, Text{txt, fontSize, WHITE});
    registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}

void TextPlugin::AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                                const float fontSize, const Position& position) {
    registry.AddComponent(entity, Text{txt, fontSize, WHITE});
    registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
    if (!registry.HasComponent<Position>(entity)) {
        registry.AddComponent(entity, Position{});
    }
}
} // namespace Engine
