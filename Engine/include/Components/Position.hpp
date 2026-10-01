#ifndef ENGINE_COMPONENTS_POSITION_HPP_
#define ENGINE_COMPONENTS_POSITION_HPP_

namespace Engine {
/**
 * @struct Position
 * @brief A basic component representing a 2D position.
 *
 * Holds current (x, y), previous-frame (prevX, prevY), and @ref moved.
 */
struct Position {
    float x;
    float y;
    float prevX;
    float prevY;

    /** @brief True when (x, y) differs from (prevX, prevY); updated by @ref TransformSystem. */
    bool moved;

    /**
     * @brief Constructor for Position.
     * @param x The initial X position.
     * @param y The initial Y position.
     */
    Position(float x = 0.0f, float y = 0.0f)
        : x(x), y(y), prevX(x), prevY(y), moved(false) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_POSITION_HPP_
