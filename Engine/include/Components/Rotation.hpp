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

    Rotation(float angle = 0.0f) : angle(angle) {}
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_ROTATION_HPP_
