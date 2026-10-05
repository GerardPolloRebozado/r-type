//
// Created by Ana Pallarés on 30/09/2026.
//

#ifndef G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP

#include <raylib-cpp.hpp>
#include <string>

namespace Engine {
    /**
     * @class Sprite
     * @brief Engine wrapper for a visual sprite, utilizing raylib::Texture to handle rendering, scaling, and rotation.
     */
    class Sprite {
    private:
        raylib::Texture _texture;
        raylib::Rectangle _sourceRect;
        raylib::Vector2 _scale;
        float _rotation;

    public:
        /**
         * @brief Constructs a new Sprite object by loading a texture from a file.
         * @param path The file path to the image file.
         */
        Sprite(const std::string &path);

        /**
         * @brief Draws the sprite to the screen at the given position using its current properties (scale, rotation, source rect).
         * @param position The world-space position to draw the sprite at.
         */
        void Draw(const raylib::Vector2& position);

        /**
         * @brief Gets the underlying raylib texture.
         * @return Constant reference to the raylib::Texture.
         */
        const raylib::Texture& GetTexture() const;
        
        /**
         * @brief Gets the source rectangle used for drawing a specific part of the texture (useful for sprite sheets).
         * @return Constant reference to the source raylib::Rectangle.
         */
        const raylib::Rectangle& GetSourceRect() const;

        /**
         * @brief Sets the source rectangle used for drawing a specific part of the texture.
         * @param rect The new source rectangle.
         */
        void SetSourceRect(const raylib::Rectangle& rect);

        /**
         * @brief Gets the scale factor of the sprite.
         * @return Constant reference to the scale vector.
         */
        const raylib::Vector2& GetScale() const;

        /**
         * @brief Sets the scale factor of the sprite.
         * @param scale The new scale vector.
         */
        void SetScale(const raylib::Vector2& scale);

        /**
         * @brief Gets the rotation angle of the sprite in degrees.
         * @return The rotation angle.
         */
        float GetRotation() const;

        /**
         * @brief Sets the rotation angle of the sprite.
         * @param rot The new rotation angle in degrees.
         */
        void SetRotation(float rot);
    };
}

#endif // G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP
