//
// Created by Ana Pallarés on 06/10/2026.
//

#include "EditorConfig.hpp"

namespace Editor {
std::string EditorConfig::GetEntityName(Engine::Entity entity) const {
    if (auto it = m_EntityNames.find(entity); it != m_EntityNames.end()) {
        return it->second;
    }
    return "Entity " + std::to_string(entity);
}
}