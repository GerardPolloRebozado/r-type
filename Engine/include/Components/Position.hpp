#ifndef ENGINE_COMPONENTS_POSITION_HPP_
#define ENGINE_COMPONENTS_POSITION_HPP_

namespace Engine {
/**
 * @struct Position
 * @brief A basic component representing a 2D position (x, y) plus a draw layer (z).
 *
 * Holds current (x, y, z), previous-frame (prevX, prevY, prevZ), @ref moved, and @ref relative.
 * Stored x/y are always world coordinates; @ref relative only gates @ref HierarchySystem delta follow.
 */
struct Position {
    float x;
    float y;
    /** @brief Draw layer: entities with a higher z are drawn on top of lower ones. */
    int z;
    float prevX;
    float prevY;
    int prevZ;

    /** @brief True when (x, y, z) differs from (prevX, prevY, prevZ); updated by @ref PositionSystem. */
    bool moved;

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
     * @param z The initial draw layer.
     */
    Position(float x = 0.0f, float y = 0.0f, int z = 0)
        : x(x), y(y), z(z), prevX(x), prevY(y), prevZ(z), moved(false), relative(false) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_POSITION_HPP_
