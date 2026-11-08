#ifndef AUTHOR_COMMANDSINTERNAL_HPP_
#define AUTHOR_COMMANDSINTERNAL_HPP_

#include "Author/Commands.hpp"
#include "Author/Protocol.hpp"
#include "Author/Session.hpp"
#include "UI/UiCatalog.hpp"
#include "UI/UiComponent.hpp"
#include <filesystem>
#include <glaze/glaze.hpp>
#include <string>
#include <vector>

namespace Author {

struct LiveBinding {
    Engine::Registry* registry = nullptr;
    Serialization::SceneConfig* config = nullptr;
    std::filesystem::path projectFolder;
    std::filesystem::path scenePath;
    std::vector<ScriptFileChange>* files = nullptr;
};

LiveBinding* CurrentLiveBinding();

glz::generic MakeResult(bool ok, const std::string& error);
std::string OpenSession(const AuthorRequest& request, Session& session);
std::string FieldTypeName(Engine::UiFieldType type);
std::string ApplyUiProps(Engine::UiComponent& ui, const glz::generic& props);
std::string ApplyUiAction(Engine::UiAction& action, const glz::generic& value, const char* label);
std::string SetComponent(Engine::Registry& registry, Engine::Entity entity,
                         const std::string& component, const glz::generic& fields);
std::string AllowedComponents();
std::vector<std::string> ComponentNames(Engine::Registry& registry, Engine::Entity entity);
std::string ValidateLuaFile(const std::filesystem::path& path);
std::filesystem::path FindEngineRoot(const std::string& engineRoot);
std::string ReadLuaDoc(const std::filesystem::path& engineRoot, const std::string& topic);

glz::generic ProjectSummary(const AuthorRequest& request);
glz::generic SceneOutline(const AuthorRequest& request);
glz::generic ListUiTemplates(const AuthorRequest& request);
glz::generic ListAssets(const AuthorRequest& request);
glz::generic ListPrefabs(const AuthorRequest& request);
glz::generic LuaApi(const AuthorRequest& request);
glz::generic AddUiScreen(const AuthorRequest& request);
glz::generic AddUiWidget(const AuthorRequest& request);
glz::generic SetUiProps(const AuthorRequest& request);
glz::generic PlacePrefab(const AuthorRequest& request);
glz::generic CreateEntityCmd(const AuthorRequest& request);
glz::generic SetComponentCmd(const AuthorRequest& request);
glz::generic GetComponentCmd(const AuthorRequest& request);
glz::generic AttachScript(const AuthorRequest& request);
glz::generic AddWave(const AuthorRequest& request);
glz::generic SetWave(const AuthorRequest& request);
glz::generic RemoveWave(const AuthorRequest& request);
glz::generic CreatePrefab(const AuthorRequest& request);
glz::generic SetPrefabComponent(const AuthorRequest& request);
glz::generic ValidateScene(const AuthorRequest& request);
glz::generic ValidateScript(const AuthorRequest& request);

} // namespace Author

#endif // AUTHOR_COMMANDSINTERNAL_HPP_
