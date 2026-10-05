#include "Systems/TagSystem.hpp"
#include <algorithm>

namespace Engine {
void TagSystem::Add(Tags& tags, std::string_view tag) {
    if (Has(tags, tag))
        return;
    tags.values.emplace_back(tag);
}

void TagSystem::Add(Registry& registry, Entity entity, std::string_view tag) {
    Add(registry.GetComponent<Tags>(entity), tag);
}

void TagSystem::Remove(Tags& tags, std::string_view tag) {
    std::vector<std::string>::iterator found = std::find(tags.values.begin(), tags.values.end(), tag);

    if (found == tags.values.end())
        return;
    tags.values.erase(found);
}

void TagSystem::Remove(Registry& registry, Entity entity, std::string_view tag) {
    Remove(registry.GetComponent<Tags>(entity), tag);
}

bool TagSystem::Has(const Tags& tags, std::string_view tag) {
    std::vector<std::string>::const_iterator found =
        std::find(tags.values.begin(), tags.values.end(), tag);

    return found != tags.values.end();
}

bool TagSystem::Has(Registry& registry, Entity entity, std::string_view tag) {
    if (!registry.HasComponent<Tags>(entity))
        return false;
    return Has(registry.GetComponent<Tags>(entity), tag);
}

std::vector<Entity> TagSystem::EntitiesWithTag(Registry& registry, std::string_view tag) {
    std::vector<Entity> matches;
    size_t count = registry.GetEntityCount();

    for (Entity entity = 0; entity < count; ++entity) {
        if (Has(registry, entity, tag))
            matches.push_back(entity);
    }
    return matches;
}
} // namespace Engine
