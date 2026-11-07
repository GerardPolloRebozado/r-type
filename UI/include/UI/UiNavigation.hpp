#ifndef UI_UINAVIGATION_HPP_
#define UI_UINAVIGATION_HPP_

#include <string>
#include <vector>

namespace Engine {

/**
 * @brief Page shown by the UI and the pages visited before it.
 *
 * Registered by UiPlugin. GoToPage pushes current onto history, Back pops it.
 * Reset when a scene is unloaded.
 */
struct UiNavigation {
    std::string current;
    std::vector<std::string> history;
    /** @brief Set current to the start page on the first draw. */
    bool started = false;
};

} // namespace Engine

#endif // UI_UINAVIGATION_HPP_
