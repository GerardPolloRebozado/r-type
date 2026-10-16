#include "EntitySubtree.hpp"
#include "Components/Name.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
#include "EditorConfig.hpp"
#include "GlazeSerializer.hpp"
#include "Logging/Logger.hpp"
#include "Systems/HierarchySystem.hpp"
#include <cctype>
#include <unordered_set>
#include <vector>

namespace Editor {
struct PrefabDocument {
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> entities;
};

namespace {
std::vector<Serialization::GlazeSerializer::EntitySnapshot>& Clipboard() {
    static std::vector<Serialization::GlazeSerializer::EntitySnapshot> clipboard;
    return clipboard;
}

void CollectSubtree(Engine::Registry& registry, Engine::Entity entity,
                    Serialization::GlazeSerializer& serializer,
                    std::vector<Serialization::GlazeSerializer::EntitySnapshot>& out,
                    std::unordered_set<Engine::Entity>& seen, int depth) {
    if (depth > 64 || !seen.insert(entity).second || !registry.IsEntityAlive(entity)) {
        return;
    }
    out.push_back(serializer.CaptureEntity(registry, entity));
    for (Engine::Entity child : Engine::HierarchySystem::FindDirectChildren(registry, entity)) {
        CollectSubtree(registry, child, serializer, out, seen, depth + 1);
    }
}

std::vector<Serialization::GlazeSerializer::EntitySnapshot>
CaptureSubtree(Engine::Registry& registry, Engine::Entity root) {
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> snapshots;
    std::unordered_set<Engine::Entity> seen;
    Serialization::GlazeSerializer serializer;
    CollectSubtree(registry, root, serializer, snapshots, seen, 0);
    return snapshots;
}

std::string SanitizePrefabBase(const std::string& name) {
    std::string base;
    for (unsigned char c : name) {
        if (std::isalnum(c) || c == '-' || c == '_') {
            base.push_back(static_cast<char>(c));
        } else if (c == ' ') {
            base.push_back('_');
        }
    }
    if (base.empty()) {
        base = "Entity";
    }
    return base;
}

std::filesystem::path UniquePrefabPath(const std::filesystem::path& directory,
                                       const std::string& base) {
    const std::filesystem::path first = directory / (base + ".prefab");
    if (!std::filesystem::exists(first)) {
        return first;
    }
    for (int suffix = 2; suffix < 1000; ++suffix) {
        const std::filesystem::path candidate =
            directory / (base + "_" + std::to_string(suffix) + ".prefab");
        if (!std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

void SetPrefabLink(Engine::Registry& registry, Engine::Entity entity, const std::string& path) {
    registry.RegisterComponent<Engine::PrefabInstance>();
    if (registry.HasComponent<Engine::PrefabInstance>(entity)) {
        registry.GetComponent<Engine::PrefabInstance>(entity).prefabPath = path;
    } else {
        registry.AddComponent<Engine::PrefabInstance>(entity, Engine::PrefabInstance::Create(path));
    }
}

// "enemy1(2)" shares the base "enemy1", so the next copy is "enemy1(3)"
// rather than "enemy1(2)(1)".
std::string NameBase(const std::string& name) {
    if (name.size() < 3 || name.back() != ')') {
        return name;
    }
    const std::size_t open = name.rfind('(');
    if (open == std::string::npos || open == 0 || open + 1 >= name.size() - 1) {
        return name;
    }
    for (std::size_t i = open + 1; i + 1 < name.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
            return name;
        }
    }
    return name.substr(0, open);
}

void AssignUniqueNames(Engine::Registry& registry,
                       std::vector<Serialization::GlazeSerializer::EntitySnapshot>& snapshots) {
    if (!registry.IsComponentRegistered<Engine::Name>()) {
        return;
    }
    std::unordered_set<std::string> used;
    for (std::size_t i = 0; i < registry.GetEntityCount(); ++i) {
        const Engine::Entity entity = i;
        if (registry.IsEntityAlive(entity) && registry.HasComponent<Engine::Name>(entity)) {
            used.insert(registry.GetComponent<Engine::Name>(entity).value);
        }
    }
    for (auto& snapshot : snapshots) {
        if (!snapshot.name.has_value()) {
            continue;
        }
        std::string& name = snapshot.name->value;
        if (!used.contains(name)) {
            used.insert(name);
            continue;
        }
        const std::string base = NameBase(name);
        for (int suffix = 1; suffix < 10000; ++suffix) {
            const std::string candidate = base + "(" + std::to_string(suffix) + ")";
            if (!used.contains(candidate)) {
                name = candidate;
                used.insert(name);
                break;
            }
        }
    }
}
} // namespace

void EntitySubtree::Copy(Engine::Registry& registry, Engine::Entity root) {
    if (!registry.IsEntityAlive(root)) {
        return;
    }
    Clipboard() = CaptureSubtree(registry, root);
}

bool EntitySubtree::CanPaste() {
    return !Clipboard().empty();
}

Engine::Entity EntitySubtree::Paste(Engine::Registry& registry) {
    if (Clipboard().empty()) {
        return static_cast<Engine::Entity>(-1);
    }
    Serialization::GlazeSerializer serializer;
    std::vector<Serialization::GlazeSerializer::EntitySnapshot> snapshots = Clipboard();
    AssignUniqueNames(registry, snapshots);
    return serializer.SpawnEntities(registry, snapshots);
}

std::string EntitySubtree::SavePrefab(Engine::Registry& registry, Engine::Entity root,
                                      const std::filesystem::path& directory) {
    if (!registry.IsEntityAlive(root) || !std::filesystem::is_directory(directory)) {
        log_error("Failed to save prefab: no entity or folder");
        return "";
    }

    std::vector<Serialization::GlazeSerializer::EntitySnapshot> snapshots =
        CaptureSubtree(registry, root);
    if (snapshots.empty()) {
        log_error("Failed to save prefab: empty subtree");
        return "";
    }

    // The source parent is a scene link. The file's root stands alone, and the
    // live entity keeps the parent it already has.
    snapshots.front().parent.reset();
    if (snapshots.front().position.has_value()) {
        snapshots.front().position->relative = false;
    }
    snapshots.front().prefab.reset();

    const std::filesystem::path file =
        UniquePrefabPath(directory, SanitizePrefabBase(Engine::GetEntityName(registry, root)));
    if (file.empty()) {
        log_error("Failed to save prefab: no free file name in " + directory.string());
        return "";
    }

    PrefabDocument document;
    document.entities = std::move(snapshots);
    Serialization::GlazeSerializer serializer;
    if (!serializer.SerializeToFile(file.string(), document)) {
        log_error("Failed to write prefab: " + file.string());
        return "";
    }

    const std::string relative = ToProjectRelativePath(registry, file.string());
    // Marker only. Changing the file later does not update this entity.
    SetPrefabLink(registry, root, relative);
    log_info("Saved prefab " + relative);
    return relative;
}

Engine::Entity EntitySubtree::SpawnPrefab(Engine::Registry& registry,
                                          const std::string& prefabPath) {
    const std::filesystem::path resolved = ResolveAssetPath(registry, prefabPath);
    PrefabDocument document;
    Serialization::GlazeSerializer serializer;
    if (!serializer.DeserializeFromFile(resolved.string(), document) || document.entities.empty()) {
        log_error("Failed to read prefab: " + prefabPath);
        return static_cast<Engine::Entity>(-1);
    }

    AssignUniqueNames(registry, document.entities);
    const Engine::Entity root = serializer.SpawnEntities(registry, document.entities);
    if (root == static_cast<Engine::Entity>(-1) || !registry.IsEntityAlive(root)) {
        return static_cast<Engine::Entity>(-1);
    }
    const std::string relative = ToProjectRelativePath(registry, prefabPath);
    SetPrefabLink(registry, root, relative);
    return root;
}
} // namespace Editor
