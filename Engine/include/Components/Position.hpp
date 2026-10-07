#ifndef ENGINE_COMPONENTS_POSITION_HPP_
#define ENGINE_COMPONENTS_POSITION_HPP_

namespace Engine {
/**
 * @struct Position
 * @brief A basic component representing a 2D position.
 *
 * Holds current (x, y), previous-frame (prevX, prevY), @ref moved, and @ref relative.
 * Stored x/y are always world coordinates; @ref relative only gates @ref HierarchySystem delta follow.
 */
struct Position {
    float x;
    float y;
    float prevX;
    float prevY;

    /** @brief True when (x, y) differs from (prevX, prevY); updated by @ref PositionSystem. */
    bool moved;

    /**
     * @brief When true and the entity has @ref Parent, @ref HierarchySystem applies the
     *        parent's frame delta to this position.
     */
    bool relative;

    /**
     * @brief Constructor for Position.
     * @param x The initial X position.
     * @param y The initial Y position.
     */
    Position(float x = 0.0f, float y = 0.0f)
        : x(x), y(y), prevX(x), prevY(y), moved(false), relative(false) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_POSITION_HPP_
