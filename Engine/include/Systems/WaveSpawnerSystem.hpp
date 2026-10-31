#ifndef ENGINE_SYSTEMS_WAVESPAWNERSYSTEM_HPP_
#define ENGINE_SYSTEMS_WAVESPAWNERSYSTEM_HPP_

#include "ECS/System.hpp"

namespace Engine {
class Registry;

/**
 * @class WaveSpawnerSystem
 * @brief Spawns scene waves from the @ref WaveSchedule resource.
 *
 * A wave spawns once the active camera's world X reaches its trigger. The
 * system replaces the old Lua wave-spawner script, so waves are authored in
 * the editor's Waves panel and saved with the scene.
 */
class WaveSpawnerSystem : public System {
  public:
    void Update(Registry& registry, float dt) override;

  private:
    float m_elapsed = 0.0f;
};

} // namespace Engine

#endif // ENGINE_SYSTEMS_WAVESPAWNERSYSTEM_HPP_
