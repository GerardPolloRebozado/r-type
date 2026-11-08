#include "Author/Session.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Components/Animation.hpp"
#include "Components/AudioSource.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/Input.hpp"
#include "Components/Name.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/Opacity.hpp"
#include "Components/Parent.hpp"
#include "Components/Position.hpp"
#include "Components/PrefabInstance.hpp"
#include "Components/Rotation.hpp"
#include "Components/ShapeComponent.hpp"
#include "Components/SignalConnections.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/Text.hpp"
#include "ProjectFile.hpp"
#include "Resources/WaveSchedule.hpp"
#include "RuntimePrefab.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "UI/UiComponent.hpp"
#include <cctype>
#include <utility>

namespace Author {
namespace {

bool NameTaken(Engine::Registry& registry, const std::string& name, Engine::Entity skip)
{
    std::size_t index = 0;
    Engine::Entity entity = 0;

    if (!registry.IsComponentRegistered<Engine::Name>()) {
        return false;
    }
    for (index = 1; index < registry.GetEntityCount(); ++index) {
        entity = Engine::Entity{index};
        if (entity == skip || !registry.IsEntityAlive(entity) ||
            !registry.HasComponent<Engine::Name>(entity)) {
            continue;
        }
        if (registry.RequireComponent<Engine::Name>(entity).value == name) {
            return true;
        }
    }
    return false;
}

std::string NameBase(const std::string& name)
{
    std::size_t open = 0;
    std::size_t index = 0;

    if (name.size() < 3 || name.back() != ')') {
        return name;
    }
    open = name.rfind('(');
    if (open == std::string::npos || open == 0 || open + 1 >= name.size() - 1) {
        return name;
    }
    for (index = open + 1; index + 1 < name.size(); ++index) {
        if (!std::isdigit(static_cast<unsigned char>(name[index]))) {
            return name;
        }
    }
    return name.substr(0, open);
}

} // namespace

void RegisterAuthorComponents(Engine::Registry& registry)
{
    registry.RegisterComponent<Engine::Name>();
    registry.RegisterComponent<Engine::Position>();
    registry.RegisterComponent<Engine::Rotation>();
    registry.RegisterComponent<Engine::SpriteComponent>();
    registry.RegisterComponent<Engine::ShapeComponent>();
    registry.RegisterComponent<Engine::Text>();
    registry.RegisterComponent<Engine::ColliderComponent>();
    registry.RegisterComponent<Engine::Parent>();
    registry.RegisterComponent<Engine::Input>();
    registry.RegisterComponent<Engine::AudioSource>();
    registry.RegisterComponent<Engine::Opacity>();
    registry.RegisterComponent<Engine::CameraComponent>();
    registry.RegisterComponent<Engine::ScriptComponent>();
    registry.RegisterComponent<Engine::SignalConnections>();
    registry.RegisterComponent<Engine::UiComponent>();
    registry.RegisterComponent<Engine::NetworkComponent>();
    registry.RegisterComponent<Engine::Animation>();
    registry.RegisterComponent<Engine::PrefabInstance>();
}

std::filesystem::path ResolveProjectFolder(const std::string& folder)
{
    std::error_code error;
    std::filesystem::path path = folder.empty() ? std::filesystem::current_path(error)
                                                : std::filesystem::path(folder);
    if (error) {
        return {};
    }
    return std::filesystem::absolute(path, error).lexically_normal();
}

std::filesystem::path ResolveScenePath(const std::filesystem::path& projectFolder,
                                       const std::string& scenePath)
{
    std::error_code error;
    std::filesystem::path path;

    if (scenePath.empty()) {
        return Serialization::StartupScenePath(projectFolder);
    }
    path = scenePath;
    if (!path.is_absolute()) {
        path = projectFolder / path;
    }
    return std::filesystem::absolute(path, error).lexically_normal();
}

std::optional<Engine::Entity> FindEntity(Engine::Registry& registry, const std::string& key)
{
    bool numeric = !key.empty();
    Engine::Entity id = 0;
    std::size_t index = 0;
    Engine::Entity entity = 0;

    if (key.empty()) {
        return std::nullopt;
    }
    for (char letter : key) {
        if (!std::isdigit(static_cast<unsigned char>(letter))) {
            numeric = false;
            break;
        }
    }
    if (numeric) {
        id = static_cast<Engine::Entity>(std::stoull(key));
        if (registry.IsEntityAlive(id)) {
            return id;
        }
    }
    if (!registry.IsComponentRegistered<Engine::Name>()) {
        return std::nullopt;
    }
    for (index = 1; index < registry.GetEntityCount(); ++index) {
        entity = Engine::Entity{index};
        if (!registry.IsEntityAlive(entity) || !registry.HasComponent<Engine::Name>(entity)) {
            continue;
        }
        if (registry.RequireComponent<Engine::Name>(entity).value == key) {
            return entity;
        }
    }
    return std::nullopt;
}

std::string UniqueEntityName(Engine::Registry& registry, const std::string& base,
                             Engine::Entity skip)
{
    std::string name = base.empty() ? "Entity" : base;
    std::string stem;
    std::string candidate;
    int suffix = 1;

    if (!NameTaken(registry, name, skip)) {
        return name;
    }
    stem = NameBase(name);
    candidate = stem + "(1)";
    while (suffix < 10000 && NameTaken(registry, candidate, skip)) {
        ++suffix;
        candidate = stem + "(" + std::to_string(suffix) + ")";
    }
    return candidate;
}

std::string EntityDisplayName(Engine::Registry& registry, Engine::Entity entity)
{
    return Engine::GetEntityName(registry, entity);
}

Session::Session() : m_Registry(&m_Owned), m_Config(&m_OwnedConfig)
{
}

std::string Session::Attach(Engine::Registry& registry, Serialization::SceneConfig& config,
                            const std::filesystem::path& projectFolder,
                            const std::filesystem::path& scenePath)
{
    m_Registry = &registry;
    m_Config = &config;
    m_ProjectFolder = projectFolder;
    m_ScenePath = scenePath;
    m_Open = true;
    RegisterAuthorComponents(*m_Registry);
    Serialization::RegisterRuntimePrefabSpawner(*m_Registry);
    if (!m_ProjectFolder.empty()) {
        if (!m_Registry->HasResource<Engine::ProjectContext>()) {
            m_Registry->AddResource<Engine::ProjectContext>(m_ProjectFolder.string());
        } else {
            m_Registry->GetResource<Engine::ProjectContext>().projectFolder = m_ProjectFolder.string();
        }
    }
    return {};
}

std::string Session::Open(const std::filesystem::path& projectFolder,
                          const std::filesystem::path& scenePath)
{
    std::error_code error;

    m_Registry = &m_Owned;
    m_Config = &m_OwnedConfig;
    m_ProjectFolder = projectFolder;
    m_ScenePath = scenePath;
    if (m_ProjectFolder.empty() || !std::filesystem::is_directory(m_ProjectFolder, error)) {
        return "projectFolder is not a directory";
    }
    if (!std::filesystem::is_regular_file(m_ScenePath, error) || error) {
        return "scene not found: " + m_ScenePath.string();
    }

    RegisterAuthorComponents(*m_Registry);
    m_Config->m_ProjectFolder = m_ProjectFolder.string();
    m_Config->m_InitialScenePath = m_ScenePath.string();
    if (!m_Registry->HasResource<Engine::ProjectContext>()) {
        m_Registry->AddResource<Engine::ProjectContext>(m_ProjectFolder.string());
    } else {
        m_Registry->GetResource<Engine::ProjectContext>().projectFolder = m_ProjectFolder.string();
    }
    Serialization::RegisterRuntimePrefabSpawner(*m_Registry);
    if (!m_Serializer.Deserialize(m_ScenePath.string(), *m_Registry, *m_Config)) {
        return "failed to deserialize scene: " + m_ScenePath.string();
    }
    m_Config->m_ProjectFolder = m_ProjectFolder.string();
    m_Config->m_InitialScenePath = m_ScenePath.string();
    m_Registry->GetResource<Engine::ProjectContext>().projectFolder = m_ProjectFolder.string();
    if (m_Registry->HasResource<Engine::WaveSchedule>()) {
        m_Config->m_Waves = m_Registry->GetResource<Engine::WaveSchedule>().waves;
    }
    m_Open = true;
    return {};
}

std::string Session::Save()
{
    std::filesystem::path temp = m_ScenePath;
    Engine::Registry check;
    Serialization::SceneConfig checkConfig;
    Serialization::SceneSerializer checkSerializer;
    std::error_code error;
    std::uintmax_t size = 0;

    if (!m_Open || m_ScenePath.empty()) {
        return "no scene is open";
    }
    if (m_Registry->HasResource<Engine::WaveSchedule>()) {
        m_Registry->GetResource<Engine::WaveSchedule>().waves = m_Config->m_Waves;
    }
    m_Config->m_ProjectFolder = m_ProjectFolder.string();
    temp += ".author-tmp";
    m_Serializer.Serialize(temp.string(), *m_Registry, *m_Config);
    size = std::filesystem::file_size(temp, error);
    if (error || size == 0) {
        std::filesystem::remove(temp, error);
        return "failed to serialize scene";
    }
    RegisterAuthorComponents(check);
    checkConfig.m_ProjectFolder = m_ProjectFolder.string();
    if (!checkSerializer.Deserialize(temp.string(), check, checkConfig)) {
        std::filesystem::remove(temp, error);
        return "validate_scene failed after write";
    }
    std::filesystem::rename(temp, m_ScenePath, error);
    if (error) {
        error.clear();
        std::filesystem::copy_file(temp, m_ScenePath,
                                   std::filesystem::copy_options::overwrite_existing, error);
        std::filesystem::remove(temp);
        if (error) {
            return "failed to replace scene file: " + error.message();
        }
    }
    return {};
}

Engine::Registry& Session::Registry()
{
    return *m_Registry;
}

Serialization::SceneConfig& Session::Config()
{
    return *m_Config;
}

const std::filesystem::path& Session::ProjectFolder() const
{
    return m_ProjectFolder;
}

const std::filesystem::path& Session::ScenePath() const
{
    return m_ScenePath;
}

} // namespace Author
