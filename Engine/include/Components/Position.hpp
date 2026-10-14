#ifndef ENGINE_COMPONENTS_POSITION_HPP_
#define ENGINE_COMPONENTS_POSITION_HPP_

namespace Engine {
/**
 * @struct Position
 * @brief A basic component representing a 2D position.
 *
 * Holds current (x, y), previous-frame (prevX, prevY), @ref moved, and @ref relative.
 * Stored x/y are always world coordinates; @ref relative only gates @ref HierarchySystem delta
 * follow.
 */
struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float prevX = 0.0f;
    float prevY = 0.0f;

    /** @brief True when (x, y) differs from (prevX, prevY); updated by @ref PositionSystem. */
    bool moved = false;

    /**
     * @brief When true and the entity has @ref Parent, @ref HierarchySystem applies the
     *        parent's frame delta to this position.
     */
    bool relative = false;

    /**
     * @brief Creates a Position. Previous-frame coordinates start equal to the
     *        current ones so the entity is not reported as moved on spawn.
     * @param x The initial X position.
     * @param y The initial Y position.
     */
    static Position Create(float x = 0.0f, float y = 0.0f) {
        Position pos;
        pos.x = x;
        pos.y = y;
        pos.prevX = x;
        pos.prevY = y;
        return pos;
    }
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_POSITION_HPP_
