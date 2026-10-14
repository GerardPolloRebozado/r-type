#ifndef ENGINE_COMPONENTS_ROTATION_HPP_
#define ENGINE_COMPONENTS_ROTATION_HPP_

namespace Engine {

/**
 * @struct Rotation
 * @brief Component representing a 2D rotation.
 */
struct Rotation {
    /** @brief Rotation angle in degrees. */
    float angle = 0.0f;

    static Rotation Create(float angle = 0.0f) {
        Rotation rot;
        rot.angle = angle;
        return rot;
    }
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_ROTATION_HPP_
