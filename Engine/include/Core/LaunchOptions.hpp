#ifndef ENGINE_CORE_LAUNCHOPTIONS_HPP_
#define ENGINE_CORE_LAUNCHOPTIONS_HPP_

#include <string>
#include <vector>

namespace Engine {

/**
 * @brief Command-line arguments handed to the game for things like launching a client or a server
 */
struct LaunchOptions {
    std::vector<std::string> args;
};

} // namespace Engine

#endif // ENGINE_CORE_LAUNCHOPTIONS_HPP_
