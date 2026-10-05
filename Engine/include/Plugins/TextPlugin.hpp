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
/**
 * @class TextPlugin
 * @brief Bundles @ref Text component registration and @ref TextSystem into the ECS.
 *
 * Provides convenient helper methods to attach text components to entities with
 * automatic @ref Position validation and default values.
 */
class TextPlugin : public IPlugin {
  public:
    /**
     * @brief Registers the @ref Text component and @ref TextSystem with the registry.
     * @param registry Main ECS registry.
     */
    void Init(Registry& registry) override;

    /**
     * @brief Attaches a default @ref Text component to an entity.
     *
     * Automatically adds a default @ref Position component if the entity does not already possess
     * one.
     * @param registry Main ECS registry.
     * @param entity Target entity to attach text to.
     * @param txt Text content string to display.
     */
    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt);

    /**
     * @brief Attaches a @ref Text component with a specified font size to an entity.
     *
     * Automatically adds a default @ref Position component if the entity does not already possess
     * one.
     * @param registry Main ECS registry.
     * @param entity Target entity to attach text to.
     * @param txt Text content string to display.
     * @param fontSize Size of the font in pixels.
     */
    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize);

    /**
     * @brief Attaches a @ref Text component with a specified font size and explicit position.
     * @param registry Main ECS registry.
     * @param entity Target entity to attach text to.
     * @param txt Text content string to display.
     * @param fontSize Size of the font in pixels.
     * @param position World position for rendering the text.
     */
    static void AttachToEntity(Registry& registry, const Entity entity, const std::string& txt,
                               float fontSize, const Position& position);
};
} // namespace Engine
#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXTPLUGIN_HPP
