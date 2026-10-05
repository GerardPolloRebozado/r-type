//
// Created by Ana Pallarés on 02/10/2026.
//

#include "Assets/Sprites.hpp"
#include <cmath>
#include <stdexcept>

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
    if (rect.width == 0.0f || rect.height == 0.0f)
        throw std::invalid_argument("Sprite::SetSourceRect: width and height must be non-zero");
    float texWidth = static_cast<float>(_texture.GetWidth());
    float texHeight = static_cast<float>(_texture.GetHeight());
    if (rect.x < 0.0f || rect.y < 0.0f || rect.x + std::fabs(rect.width) > texWidth ||
        rect.y + std::fabs(rect.height) > texHeight)
        throw std::invalid_argument("Sprite::SetSourceRect: rectangle is outside the texture");
    _sourceRect = rect;
}

const raylib::Vector2& Sprite::GetScale() const {
    return _scale;
}

void Sprite::SetScale(const raylib::Vector2& scale) {
    if (!(scale.x > 0.0f) || !(scale.y > 0.0f))
        throw std::invalid_argument("Sprite::SetScale: scale components must be greater than 0");
    _scale = scale;
}

float Sprite::GetRotation() const {
    return _rotation;
}

void Sprite::SetRotation(float rot) {
    _rotation = rot;
}

} // namespace Engine