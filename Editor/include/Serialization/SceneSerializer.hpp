#pragma once

#include "ECS/Registry.hpp"
#include <string>

namespace Editor {

/**
 * @brief Handles serialization of the active Scene/Registry to and from disk.
 *
 * REASONING (JSON format):
 * JSON is somewhat easy to version control, so developers would have an easier time dealing with
 * conflicts
 *
 * JSON is human readable, so it is easy to fix by hand, if, for some reason, the editor messes up.
 *
 * Later, we can have a "scene compiler" or somethin, that turns the .scene json into a binary-like
 * format to make it way faster on a release-build.
 */
class SceneSerializer {
  public:
    SceneSerializer() = default;

    void Serialize(const std::string& filepath, Engine::Registry& registry);
    bool Deserialize(const std::string& filepath, Engine::Registry& registry);
};

} // namespace Editor
