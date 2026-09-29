#ifndef ENGINE_COMPONENTS_COMPONENTEXAMPLE_HPP_
#define ENGINE_COMPONENTS_COMPONENTEXAMPLE_HPP_

namespace Engine {
/**
 * @brief A simple example component that stores raw data.
 */
struct ComponentExample {
    float someData;
    int someState;

    ComponentExample(float data = 0.0f, int state = 0) : someData(data), someState(state) {}
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_COMPONENTEXAMPLE_HPP_
