//
// Created by Ana Pallarés on 02/10/2026.
//

#include "Assets/Sprites.hpp"

namespace Engine {
Sprite::Sprite(const std::string& path) : _texture(path) {
    _sourceRect = raylib::Rectangle(0, 0, static_cast<float>(_texture.GetWidth()),
                                    static_cast<float>(_texture.GetHeight()));
    _scale = raylib::Vector2(1, 1);
    _rotation = 0.0f;
}

void Sprite::Draw(const raylib::Vector2& position) {
    raylib::Rectangle destRect(position.x, position.y, _sourceRect.width * _scale.x,
                               _sourceRect.height * _scale.y);
    raylib::Vector2 origin(0, 0);
    _texture.Draw(_sourceRect, destRect, origin, _rotation, raylib::Color::White());
}

const raylib::Texture& Sprite::GetTexture() const {
    return _texture;
}

const raylib::Rectangle& Sprite::GetSourceRect() const {
    return _sourceRect;
}

void Sprite::SetSourceRect(const raylib::Rectangle& rect) {
    _sourceRect = rect;
}

const raylib::Vector2& Sprite::GetScale() const {
    return _scale;
}

void Sprite::SetScale(const raylib::Vector2& scale) {
    _scale = scale;
}

float Sprite::GetRotation() const {
    return _rotation;
}

void Sprite::SetRotation(float rot) {
    _rotation = rot;
}

} // namespace Engine