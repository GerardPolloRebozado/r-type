/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#include "Components/Position.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @class TextSystem
 * @brief Renders 2D text elements for entities possessing both @ref Text and @ref Position.
 *
 * Iterates through all entities with a @ref Text component. If the entity has an associated
 * @ref Position component, the text is drawn using Raylib's text rendering functions:
 * `DrawText` for default fonts or `DrawTextEx` for custom fonts.
 */
class TextSystem : public System {
  public:
    /**
     * @brief Updates and renders text for all valid text entities.
     * @param registry Main ECS registry containing entities and components.
     * @param dt Delta time for the frame in seconds (unused for static text rendering).
     */
    void Update(Registry& registry, float dt) override;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
