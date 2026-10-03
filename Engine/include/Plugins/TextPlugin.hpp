/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
#include "IPlugin.hpp"
#include "Components/Position.hpp"
#include "Components/Text.hpp"
#include "Systems/TextSystem.hpp"

namespace Engine {
    class TextPlugin : public IPlugin {
    public:
        void Init(Registry &registry) override {
            registry.RegisterComponent<Text>();
            registry.RegisterSystem<TextSystem>();
        };

        static void AttachToEntity(Registry &registry, const Entity entity, const std::string &txt) {
            registry.AddComponent(entity, Text{txt, 10.0f, WHITE});
            registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
            registry.AddComponent(entity, Position{});
        }


        static void AttachToEntity(Registry &registry, const Entity entity, const std::string &txt, float fontSize) {
            registry.AddComponent(entity, Text{txt, fontSize, WHITE});
            registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
            registry.AddComponent(entity, Position{});
        }

        static void AttachToEntity(Registry &registry, const Entity entity, const std::string &txt, float fontSize,
                                   const Position position) {
            registry.AddComponent(entity, Text{txt, fontSize, WHITE});
            registry.GetSystem<TextSystem>()->m_Entities.insert(entity);
            registry.AddComponent(entity, position);
        }
    };
}
#endif //G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
