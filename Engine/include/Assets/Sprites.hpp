//
// Created by Ana Pallarés on 30/09/2026.
//

#ifndef G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP
#define G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP

#include <raylib-cpp.hpp>
#include <string>

namespace Engine {
    class Sprite {
    private:
        raylib::Texture _texture;
        raylib::Rectangle _sourceRect;
        raylib::Vector2 _position;
        raylib::Vector2 _scale;
        float _rotation;

    public:
        Sprite(const std::string &path) {
            _texture = raylib::Texture(path);
            _sourceRect = raylib::Rectangle(0, 0, static_cast<float>(_texture.GetWidth()), static_cast<float>(_texture.GetHeight()));
            _position = raylib::Vector2(0, 0);
            _scale = raylib::Vector2(1, 1);
            _rotation = 0.0f;
        }

        void Draw() {
            raylib::Rectangle destRect(_position.x, _position.y, _sourceRect.width * _scale.x, _sourceRect.height * _scale.y);
            raylib::Vector2 origin(0, 0);
            _texture.Draw(_sourceRect, destRect, origin, _rotation, raylib::Color::White());
        }

        const raylib::Texture& GetTexture() const { return _texture; }
        
        const raylib::Rectangle& GetSourceRect() const { return _sourceRect; }
        void SetSourceRect(const raylib::Rectangle& rect) { _sourceRect = rect; }

        const raylib::Vector2& GetPosition() const { return _position; }
        void SetPosition(const raylib::Vector2& pos) { _position = pos; }

        const raylib::Vector2& GetScale() const { return _scale; }
        void SetScale(const raylib::Vector2& scale) { _scale = scale; }

        float GetRotation() const { return _rotation; }
        void SetRotation(float rot) { _rotation = rot; }
    };
}

#endif // G_ING_500_BAR_5_1_RTYPE_1_SPRITES_HPP
