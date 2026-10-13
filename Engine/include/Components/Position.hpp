#ifndef ENGINE_COMPONENTS_POSITION_HPP_
#define ENGINE_COMPONENTS_POSITION_HPP_

namespace Engine {
/**
 * @struct Position
 * @brief A basic component representing a position (x, y) plus a z index.
 *
 * Holds current (x, y, z), previous-frame (prevX, prevY, prevZ), @ref moved, and @ref relative.
 * Stored x/y/z are always world coordinates; @ref relative only gates @ref HierarchySystem delta follow.
 */
struct Position {
    float x;
    float y;
    float z;
    float prevX;
    float prevY;
    float prevZ;

    /** @brief True when (x, y, z) differs from (prevX, prevY, prevZ); updated by @ref PositionSystem. */
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
     * @param z The initial Z position.
     */
    Position(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : x(x), y(y), z(z), prevX(x), prevY(y), prevZ(z), moved(false), relative(false) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_POSITION_HPP_
