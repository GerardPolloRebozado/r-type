#include "Components/ShapeComponent.hpp"

#include "Vector2.hpp"

namespace Engine {
std::vector<raylib::Vector2> ShapeComponent::GetShapeVectors(const Rotation& rot,
                                                             const Position& pos) const {
    const float rad = rot.angle * DEG2RAD;
    const float cosA = std::cos(rad);
    const float sinA = std::sin(rad);

    switch (this->type) {
        case ShapeType::RECTANGLE: {
            const float halfW = this->width / 2.0f;
            const float halfH = this->height / 2.0f;
            const float centerX = pos.x + halfW;
            const float centerY = pos.y + halfH;

            const Vector2 localCorners[4] = {
                {-halfW, -halfH}, {+halfW, -halfH}, {+halfW, +halfH}, {-halfW, +halfH}};

            std::vector<raylib::Vector2> rect;
            rect.reserve(4);
            for (const auto& corner : localCorners) {
                rect.push_back({centerX + (corner.x * cosA - corner.y * sinA),
                                centerY + (corner.x * sinA + corner.y * cosA)});
            }
            return rect;
        }

        case ShapeType::POLYGON: {
            std::vector<raylib::Vector2> poly;
            poly.reserve(this->sides);
            const float step = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(this->sides);
            for (int i = 0; i < this->sides; ++i) {
                const float a = rad + static_cast<float>(i) * step;
                poly.push_back(
                    {pos.x + this->radius * std::cos(a), pos.y + this->radius * std::sin(a)});
            }
            return poly;
        }

        case ShapeType::LINE: {
            const Vector2 start = {pos.x, pos.y};
            const Vector2 end = {pos.x + (this->endOffset.x * cosA - this->endOffset.y * sinA),
                                 pos.y + (this->endOffset.x * sinA + this->endOffset.y * cosA)};
            return {start, end};
        }

        case ShapeType::CIRCLE: {
            constexpr int circleSides = 24;
            std::vector<raylib::Vector2> circle;
            circle.reserve(circleSides);
            const float step = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(circleSides);
            for (int i = 0; i < circleSides; ++i) {
                const float a = rad + static_cast<float>(i) * step;
                circle.push_back(
                    {pos.x + this->radius * std::cos(a), pos.y + this->radius * std::sin(a)});
            }
            return circle;
        }

        default:
            return {};
    }
}
} // namespace Engine