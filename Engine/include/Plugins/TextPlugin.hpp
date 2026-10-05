/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextPlugin.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
#include "Components/Position.hpp"
#include "Components/Text.hpp"
#include "IPlugin.hpp"
#include "Systems/TextSystem.hpp"

namespace Engine {
class TextPlugin : public IPlugin {
  public:
    void Init(Registry& registry) override;

    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt);

    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize);

    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize, const Position position);
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
