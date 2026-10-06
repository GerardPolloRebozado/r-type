#ifndef EDITOR_EDITORCONFIG_HPP_
#define EDITOR_EDITORCONFIG_HPP_

#include "Core/Application.hpp"
#include "ECS/Entity.hpp"
#include <unordered_map>
#include <string>

namespace Editor {

struct EditorConfig : public Engine::ApplicationConfig {
    std::string m_ProjectFolder;
    std::string m_SelectedAssetPath;
    std::string m_BuildExecutableName = "Game";
    std::string m_BuildWindowTitle = "Game";
    // Editor-only entity names, never stored in the Registry
    std::unordered_map<Engine::Entity, std::string> m_EntityNames;
    /**
     * Returns the name of the given entity.
     * @param entity The entity for which to get the name.
     * @return The name of the entity.
     */
    std::string GetEntityName(Engine::Entity entity) const;
};

} // namespace Editor

#endif // EDITOR_EDITORCONFIG_HPP_
