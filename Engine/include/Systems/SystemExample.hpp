#ifndef ENGINE_SYSTEMS_SYSTEMEXAMPLE_HPP_
#define ENGINE_SYSTEMS_SYSTEMEXAMPLE_HPP_

#include "ECS/Registry.hpp"
#include "ECS/System.hpp"

namespace Engine {
/**
 * @brief A simple example system that modifies ComponentExample and fires EventsExample.
 */
class SystemExample : public System {
  public:
    void Update(Registry& registry, float dt);
};
} // namespace Engine

#endif // ENGINE_SYSTEMS_SYSTEMEXAMPLE_HPP_
