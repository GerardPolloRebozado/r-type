#include "Author/Commands.hpp"
#include "Author/CommandsInternal.hpp"
#include <fstream>

namespace Author {

glz::generic MakeResult(bool ok, const std::string& error)
{
    glz::generic result = glz::generic::object_t{};
    auto& obj = result.get_object();

    obj["apiVersion"] = kApiVersion;
    obj["ok"] = ok;
    obj["error"] = error;
    return result;
}

namespace {

LiveBinding* g_Live = nullptr;

} // namespace

LiveBinding* CurrentLiveBinding()
{
    return g_Live;
}

std::string OpenSession(const AuthorRequest& request, Session& session)
{
    std::filesystem::path project = ResolveProjectFolder(request.projectFolder);
    std::filesystem::path scene;
    LiveBinding* live = CurrentLiveBinding();

    if (live != nullptr && live->registry != nullptr && live->config != nullptr) {
        return session.Attach(*live->registry, *live->config, live->projectFolder, live->scenePath);
    }
    if (project.empty()) {
        return "projectFolder is required";
    }
    scene = ResolveScenePath(project, request.scenePath);
    return session.Open(project, scene);
}

glz::generic RunCommand(const AuthorRequest& request)
{
    if (request.apiVersion != kApiVersion) {
        return MakeResult(false, "unsupported apiVersion " + std::to_string(request.apiVersion) +
                                     " (tool is " + std::to_string(kApiVersion) + ")");
    }
    if (request.command == "project_summary") {
        return ProjectSummary(request);
    }
    if (request.command == "scene_outline") {
        return SceneOutline(request);
    }
    if (request.command == "list_ui_templates") {
        return ListUiTemplates(request);
    }
    if (request.command == "list_assets") {
        return ListAssets(request);
    }
    if (request.command == "list_prefabs") {
        return ListPrefabs(request);
    }
    if (request.command == "lua_api") {
        return LuaApi(request);
    }
    if (request.command == "add_ui_screen") {
        return AddUiScreen(request);
    }
    if (request.command == "add_ui_widget") {
        return AddUiWidget(request);
    }
    if (request.command == "set_ui_props") {
        return SetUiProps(request);
    }
    if (request.command == "place_prefab") {
        return PlacePrefab(request);
    }
    if (request.command == "create_entity") {
        return CreateEntityCmd(request);
    }
    if (request.command == "set_component") {
        return SetComponentCmd(request);
    }
    if (request.command == "attach_script") {
        return AttachScript(request);
    }
    if (request.command == "add_wave") {
        return AddWave(request);
    }
    if (request.command == "validate_scene") {
        return ValidateScene(request);
    }
    if (request.command == "validate_script") {
        return ValidateScript(request);
    }
    return MakeResult(false, "unknown command: " + request.command);
}

namespace {

bool ResultOk(const glz::generic& result)
{
    if (!result.is_object()) {
        return false;
    }
    const auto it = result.get_object().find("ok");
    return it != result.get_object().end() && it->second.is_boolean() && it->second.get<bool>();
}

void RollbackScriptFiles(std::vector<ScriptFileChange>& files, std::size_t keep)
{
    std::error_code error;

    while (files.size() > keep) {
        const ScriptFileChange change = files.back();
        files.pop_back();
        if (!change.existed) {
            std::filesystem::remove(change.path, error);
            continue;
        }
        std::ofstream out(change.path, std::ios::binary | std::ios::trunc);
        if (out) {
            out << change.previous;
        }
    }
}

} // namespace

glz::generic RunLiveCommand(const AuthorRequest& request, Engine::Registry& registry,
                            Serialization::SceneConfig& config,
                            const std::filesystem::path& projectFolder,
                            const std::filesystem::path& scenePath,
                            std::vector<ScriptFileChange>& files)
{
    LiveBinding binding;
    LiveBinding* previous = g_Live;
    std::size_t fileCount = files.size();
    glz::generic result;
    bool failed = false;

    binding.registry = &registry;
    binding.config = &config;
    binding.projectFolder = projectFolder;
    binding.scenePath = scenePath;
    binding.files = &files;
    g_Live = &binding;
    try {
        result = RunCommand(request);
        failed = !ResultOk(result);
    } catch (...) {
        g_Live = previous;
        RollbackScriptFiles(files, fileCount);
        throw;
    }
    g_Live = previous;
    if (failed) {
        RollbackScriptFiles(files, fileCount);
    }
    return result;
}

} // namespace Author
