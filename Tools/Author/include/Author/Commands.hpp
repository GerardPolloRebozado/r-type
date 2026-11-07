#ifndef AUTHOR_COMMANDS_HPP_
#define AUTHOR_COMMANDS_HPP_

#include "Author/Protocol.hpp"
#include "ECS/Registry.hpp"
#include "SceneConfig.hpp"
#include <filesystem>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>

namespace Author {

/**
 * @brief Runs one authoring command and returns a JSON object with apiVersion.
 */
/**
 * @brief One Lua file a live command created or replaced, so the editor can undo it.
 */
struct ScriptFileChange {
    std::string path;
    bool existed = false;
    std::string previous;
};

glz::generic RunCommand(const AuthorRequest& request);

/**
 * @brief Runs @p request on an open registry and does not write the scene file.
 *
 * Script files may still be created. Each one is appended to @p files.
 */
glz::generic RunLiveCommand(const AuthorRequest& request, Engine::Registry& registry,
                            Serialization::SceneConfig& config,
                            const std::filesystem::path& projectFolder,
                            const std::filesystem::path& scenePath,
                            std::vector<ScriptFileChange>& files);

/** @brief MCP tool list as a JSON array. Shared with the editor chat. */
const char* ToolCatalogJson();

/**
 * @brief MCP server on stdin/stdout. No window.
 *
 * One JSON-RPC object per line. tools/call runs @ref RunCommand.
 * @return 0 when stdin closes, 1 if a reply cannot be encoded.
 */
int RunMcpStdio();

} // namespace Author

#endif // AUTHOR_COMMANDS_HPP_
