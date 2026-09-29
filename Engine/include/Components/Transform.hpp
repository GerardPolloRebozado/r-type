#ifndef ENGINE_COMPONENTS_TRANSFORM_HPP_
#define ENGINE_COMPONENTS_TRANSFORM_HPP_

namespace Engine {
/**
 * @struct Transform
 * @brief A basic component representing a 2D position.
 *
 * This component holds the X and Y coordinates for an entity.
 */
struct Transform {
    float x;
    float y;

    /**
     * @brief Constructor for Transform.
     * @param x The initial X position.
     * @param y The initial Y position.
     */
    Transform(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_TRANSFORM_HPP_
