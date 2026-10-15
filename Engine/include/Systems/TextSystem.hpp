/*
** EPITECH PROJECT, 2026
** G-ING-500-BAR-5-1-rtype-1
** File description:
** TextSystem.hpp
*/
#ifndef G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
#include "Components/Position.hpp"
#include "ECS/LayeredRenderSystem.hpp"

namespace Engine {
/**
 * @class TextSystem
 * @brief Renders 2D text elements for entities possessing both @ref Text and @ref Position.
 *
 * Iterates through all entities with a @ref Text component. If the entity has an associated
 * @ref Position component, the text is drawn using Raylib's text rendering functions:
 * `DrawText` for default fonts or `DrawTextEx` for custom fonts.
 */
class TextSystem : public LayeredRenderSystem {
  public:
    /**
     * @brief True if @p entity has a Text and belongs to this pass
     *        (entities tagged with @ref ScreenSpace go in the screen pass).
     */
    bool CanDraw(Registry& registry, Entity entity, bool screenSpace) override;

    /**
     * @brief Draws the entity's text at its Position.
     */
    void DrawEntity(Registry& registry, Entity entity) override;
};
} // namespace Engine

#endif // G_ING_500_BAR_5_1_RTYPE_1_TEXTSYSTEM_HPP
