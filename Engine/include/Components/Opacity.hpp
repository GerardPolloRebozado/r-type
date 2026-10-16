#ifndef ENGINE_COMPONENTS_OPACITY_HPP_
#define ENGINE_COMPONENTS_OPACITY_HPP_

#include <algorithm>

namespace Engine {

/**
 * @struct Opacity
 * @brief Per-entity transparency multiplier applied at render time.
 *
 * A value of 1.0 is fully opaque, 0.0 fully transparent. Sprite, shape and
 * text renderers multiply their base alpha by the *effective* alpha: the
 * entity's own value times every ancestor's value whose @ref inheritToChildren
 * is set. Inheritance is on by default; unchecking it keeps the entity's own
 * opacity local to itself (descendants ignore that node only).
 */
struct Opacity {
    /** @brief Transparency multiplier in [0.0, 1.0]. */
    float alpha = 1.0f;

    /** @brief When true (default), this entity's opacity multiplies into descendants. */
    bool inheritToChildren = true;

    static Opacity Create(float alpha = 1.0f, bool inheritToChildren = true) {
        Opacity opacity;
        opacity.alpha = std::clamp(alpha, 0.0f, 1.0f);
        opacity.inheritToChildren = inheritToChildren;
        return opacity;
    }
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_OPACITY_HPP_
