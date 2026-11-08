#ifndef AUTHOR_SESSION_HPP_
#define AUTHOR_SESSION_HPP_

#include "ECS/Registry.hpp"
#include "SceneConfig.hpp"
#include "SceneSerializer.hpp"
#include <filesystem>
#include <optional>
#include <string>

namespace Author {

void RegisterAuthorComponents(Engine::Registry& registry);

/**
 * @brief Live registry loaded from a project scene, saved through SceneSerializer.
 */
class Session {
  public:
    Session();
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    /**
     * @brief Uses an already-open registry. Save() still writes the scene file.
     */
    std::string Attach(Engine::Registry& registry, Serialization::SceneConfig& config,
                       const std::filesystem::path& projectFolder,
                       const std::filesystem::path& scenePath);

    /**
     * @brief Registers components and loads @p scenePath from @p projectFolder.
     * @return Empty on success, otherwise an error the caller can return as JSON.
     */
    std::string Open(const std::filesystem::path& projectFolder,
                     const std::filesystem::path& scenePath);

    /**
     * @brief Writes the scene to a temp file, reloads it, then replaces the original.
     * @return Empty on success.
     */
    std::string Save();

    Engine::Registry& Registry();
    Serialization::SceneConfig& Config();
    const std::filesystem::path& ProjectFolder() const;
    const std::filesystem::path& ScenePath() const;

  private:
    Engine::Registry m_Owned;
    Serialization::SceneConfig m_OwnedConfig;
    Engine::Registry* m_Registry = nullptr;
    Serialization::SceneConfig* m_Config = nullptr;
    Serialization::SceneSerializer m_Serializer;
    std::filesystem::path m_ProjectFolder;
    std::filesystem::path m_ScenePath;
    bool m_Open = false;
};

std::filesystem::path ResolveProjectFolder(const std::string& folder);
std::filesystem::path ResolveScenePath(const std::filesystem::path& projectFolder,
                                       const std::string& scenePath);
std::optional<Engine::Entity> FindEntity(Engine::Registry& registry, const std::string& key);
std::string UniqueEntityName(Engine::Registry& registry, const std::string& base,
                             Engine::Entity skip = static_cast<Engine::Entity>(-1));
std::string EntityDisplayName(Engine::Registry& registry, Engine::Entity entity);

} // namespace Author

#endif // AUTHOR_SESSION_HPP_
