#include "Scaffold.hpp"

#include "Gallery.hpp"
#include "GalleryReflect.hpp"
#include "Logging/Logger.hpp"
#include "MultiplatformUtils.hpp"
#include "ProjectFile.hpp"
#include "ProjectSystemsBuild.hpp"
#include "SceneSerializer.hpp"

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>

namespace Editor {
namespace {

ScaffoldResult Fail(const std::string& message) {
    log_error(message);
    return ScaffoldResult{false, message, ""};
}

ScaffoldResult Succeed(const std::string& message, const std::string& scenePath) {
    log_info(message);
    return ScaffoldResult{true, message, scenePath};
}

bool IsIdentifier(const std::string& name) {
    if (name.empty()) {
        return false;
    }
    unsigned char first = static_cast<unsigned char>(name[0]);
    if (!(std::isalpha(first) || name[0] == '_')) {
        return false;
    }
    for (char ch : name) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (!(std::isalnum(c) || ch == '_')) {
            return false;
        }
    }
    return true;
}

std::string WithSuffix(const std::string& name, const std::string& suffix) {
    if (name.size() >= suffix.size() &&
        name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return name;
    }
    return name + suffix;
}

std::string WithoutSuffix(const std::string& name, const std::string& suffix) {
    if (name.size() <= suffix.size()) {
        return name;
    }
    if (name.compare(name.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return name;
    }
    return name.substr(0, name.size() - suffix.size());
}

std::string Upper(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char ch : name) {
        out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return out;
}

std::string ExecutableName(const std::string& name) {
    std::string out;
    for (char ch : name) {
        if (ch != ' ') {
            out.push_back(ch);
        }
    }
    return out;
}

bool WriteNewFile(const std::filesystem::path& path, const std::string& contents,
                  std::string& error) {
    if (std::filesystem::exists(path)) {
        error = "File already exists: " + path.string();
        return false;
    }
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    if (!out) {
        error = "Failed to write " + path.string();
        return false;
    }
    out << contents;
    if (!out) {
        error = "Failed to write " + path.string();
        return false;
    }
    return true;
}

bool WriteEmptyScene(const std::filesystem::path& scenePath, const std::filesystem::path& folder,
                     const std::string& name, const std::string& executableName,
                     std::string& error) {
    Serialization::SceneData scene;
    scene.sceneName = name;
    scene.config.windowTitle = name;
    scene.config.windowWidth = 1280;
    scene.config.windowHeight = 720;
    scene.config.targetFPS = 60;
    scene.config.m_ProjectFolder = folder.string();
    scene.config.m_BuildExecutableName = executableName;
    scene.config.m_BuildWindowTitle = name;
    scene.entities.sceneName = name;

    std::string sceneJson;
    if (glz::write_json(scene, sceneJson)) {
        error = "Failed to write " + scenePath.string();
        return false;
    }
    return WriteNewFile(scenePath, sceneJson + "\n", error);
}

std::filesystem::path ResolveOutputRoot(const std::string& outputPath) {
    std::filesystem::path root;
    if (outputPath.empty()) {
        root = std::filesystem::current_path();
    } else {
        root = std::filesystem::absolute(outputPath);
    }
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(root, error);
    if (!error) {
        root = canonical;
    }
    return root;
}

bool CMakeListsHasEngineSources(const std::filesystem::path& cmakeFile) {
    std::ifstream in(cmakeFile);
    if (!in) {
        return false;
    }
    const std::string contents((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());
    return contents.find("set(SRC_FILES") != std::string::npos &&
           contents.find("set(INC_FILES") != std::string::npos;
}

std::optional<std::filesystem::path> FindEngineCMakeLists(std::filesystem::path start) {
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(start, error);
    if (!error) {
        start = canonical;
    }

    for (std::filesystem::path dir = start;; dir = dir.parent_path()) {
        const std::filesystem::path local = dir / "CMakeLists.txt";
        if (std::filesystem::exists(local) && CMakeListsHasEngineSources(local)) {
            return local;
        }
        const std::filesystem::path nested = dir / "Engine" / "CMakeLists.txt";
        if (std::filesystem::exists(nested) && CMakeListsHasEngineSources(nested)) {
            return nested;
        }
        if (!dir.has_parent_path() || dir == dir.parent_path()) {
            break;
        }
    }
    return std::nullopt;
}

bool IsPathInside(const std::filesystem::path& child, const std::filesystem::path& parent) {
    std::error_code error;
    const std::filesystem::path resolvedChild = std::filesystem::weakly_canonical(child, error);
    const std::filesystem::path resolvedParent =
        error ? parent : std::filesystem::weakly_canonical(parent, error);
    if (resolvedChild == resolvedParent) {
        return true;
    }
    const std::filesystem::path relative =
        std::filesystem::relative(resolvedChild, resolvedParent, error);
    if (error || relative.empty()) {
        return false;
    }
    const std::string relativeText = relative.generic_string();
    return !(relativeText.rfind("..", 0) == 0);
}

bool UsesEngineLayout(const std::filesystem::path& outputRoot,
                      const std::filesystem::path& engineRoot) {
    if (outputRoot == engineRoot || IsPathInside(outputRoot, engineRoot)) {
        return true;
    }
    if (std::filesystem::exists(outputRoot / "include" / "Systems") ||
        std::filesystem::exists(outputRoot / "src" / "Systems")) {
        return true;
    }
    return false;
}

bool InsertCMakeEntries(const std::filesystem::path& cmakeFile,
                        const std::vector<std::pair<std::string, std::string>>& entries,
                        std::string& error) {
    std::ifstream in(cmakeFile);
    if (!in) {
        error = "Failed to open " + cmakeFile.string();
        return false;
    }
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    bool changed = false;
    for (const auto& entry : entries) {
        const std::string& listName = entry.first;
        const std::string& relativePath = entry.second;
        if (contents.find(relativePath) != std::string::npos) {
            continue;
        }
        const std::string marker = "set(" + listName;
        const std::size_t listPos = contents.find(marker);
        if (listPos == std::string::npos) {
            error = "Could not find " + marker + " in " + cmakeFile.string();
            return false;
        }
        const std::size_t closePos = contents.find("\n)", listPos);
        if (closePos == std::string::npos) {
            error = "Could not find the end of " + listName + " in " + cmakeFile.string();
            return false;
        }
        contents.insert(closePos + 1, "        " + relativePath + "\n");
        changed = true;
    }

    if (!changed) {
        return true;
    }

    std::ofstream out(cmakeFile);
    if (!out) {
        error = "Failed to write " + cmakeFile.string();
        return false;
    }
    out << contents;
    if (!out) {
        error = "Failed to write " + cmakeFile.string();
        return false;
    }
    return true;
}

std::string SystemHeader(const std::string& systemName) {
    const std::string guard = "ENGINE_SYSTEMS_" + Upper(systemName) + "_HPP_";
    return "#ifndef " + guard + "\n#define " + guard +
           "\n"
           "#include \"ECS/Registry.hpp\"\n"
           "#include \"ECS/System.hpp\"\n"
           "\n"
           "namespace Engine {\n"
           "/**\n"
           " * @class " +
           systemName +
           "\n"
           " * @brief Updates entities tracked by this system.\n"
           " */\n"
           "class " +
           systemName +
           " : public System {\n"
           "  public:\n"
           "    /**\n"
           "     * @brief Updates entities in @ref System::m_Entities.\n"
           "     * @param registry The ECS registry containing the components.\n"
           "     * @param dt Delta time for framerate independent movement.\n"
           "     */\n"
           "    void Update(Registry& registry, float dt) override;\n"
           "};\n"
           "} // namespace Engine\n"
           "\n"
           "#endif // " +
           guard + "\n";
}

std::string SystemSource(const std::string& systemName, const std::string& includePath) {
    return "#include \"" + includePath + systemName +
           ".hpp\"\n"
           "\n"
           "namespace Engine {\n"
           "void " +
           systemName +
           "::Update(Registry& registry, float dt) {\n"
           "    (void)registry;\n"
           "    (void)dt;\n"
           "}\n"
           "} // namespace Engine\n";
}

std::string ComponentHeader(const std::string& componentName, const std::string& systemName) {
    const std::string guard = "ENGINE_COMPONENTS_" + Upper(componentName) + "_HPP_";
    return "#ifndef " + guard + "\n#define " + guard +
           "\n"
           "\n"
           "namespace Engine {\n"
           "/**\n"
           " * @struct " +
           componentName + "\n * @brief Component data updated by @ref " + systemName +
           ".\n"
           " */\n"
           "struct " +
           componentName +
           " {\n"
           "    /** @brief Value read and written by @ref " +
           systemName +
           ". */\n"
           "    float value = 0.0f;\n"
           "};\n"
           "} // namespace Engine\n"
           "\n"
           "#endif // " +
           guard + "\n";
}

std::string PluginHeader(const std::string& pluginName, const std::string& componentName,
                         const std::string& systemName, bool projectLayout) {
    const std::string guard = "ENGINE_PLUGINS_" + Upper(pluginName) + "_HPP_";
    const std::string componentInclude =
        projectLayout ? componentName + ".hpp" : "Components/" + componentName + ".hpp";
    const std::string systemInclude =
        projectLayout ? systemName + ".hpp" : "Systems/" + systemName + ".hpp";
    return "#ifndef " + guard + "\n#define " + guard +
           "\n"
           "#include \"" +
           componentInclude +
           "\"\n"
           "#include \"Plugins/IPlugin.hpp\"\n"
           "#include \"" +
           systemInclude +
           "\"\n"
           "\n"
           "namespace Engine {\n"
           "\n"
           "/**\n"
           " * @class " +
           pluginName + "\n * @brief Bundles the " + componentName + " component and " +
           systemName +
           ".\n"
           " *\n"
           " * Call Init to register both with a registry.\n"
           " */\n"
           "class " +
           pluginName +
           " : public IPlugin {\n"
           "  public:\n"
           "    /**\n"
           "     * @brief Initializes the plugin.\n"
           "     * @param registry The main ECS registry.\n"
           "     */\n"
           "    void Init(Registry& registry) override;\n"
           "};\n"
           "} // namespace Engine\n"
           "\n"
           "#endif // " +
           guard + "\n";
}

std::string PluginSource(const std::string& pluginName, const std::string& componentName,
                         const std::string& systemName, const std::string& includePath) {
    (void)componentName;
    (void)systemName;
    return "#include \"" + includePath + pluginName +
           ".hpp\"\n"
           "\n"
           "namespace Engine {\n"
           "void " +
           pluginName +
           "::Init(Registry& registry) {\n"
           "    registry.RegisterComponent<" +
           componentName +
           ">();\n"
           "\n"
           "    registry.RegisterSystem<" +
           systemName +
           ">();\n"
           "}\n"
           "} // namespace Engine\n";
}

struct EngineFile {
    std::filesystem::path path;
    std::string contents;
    std::string cmakeList;
    std::string cmakePath;
};

ScaffoldResult WriteEngineFiles(const std::filesystem::path& cmakeFile,
                                const std::vector<EngineFile>& files) {
    if (!std::filesystem::exists(cmakeFile)) {
        return Fail("CMakeLists.txt was not found: " + cmakeFile.string());
    }

    for (const EngineFile& file : files) {
        if (std::filesystem::exists(file.path)) {
            return Fail("File already exists: " + file.path.string());
        }
    }

    for (const EngineFile& file : files) {
        std::string error;
        if (!WriteNewFile(file.path, file.contents, error)) {
            return Fail(error);
        }
    }

    std::vector<std::pair<std::string, std::string>> entries;
    entries.reserve(files.size());
    for (const EngineFile& file : files) {
        entries.push_back({file.cmakeList, file.cmakePath});
    }

    std::string error;
    if (!InsertCMakeEntries(cmakeFile, entries, error)) {
        return Fail(error);
    }

    std::string message = "Created";
    for (const EngineFile& file : files) {
        message += " " + file.path.string();
    }
    return Succeed(message, "");
}

struct ProjectFile {
    std::filesystem::path path;
    std::string contents;
    std::string cppFileName;
};

ScaffoldResult WriteProjectFiles(const std::filesystem::path& projectRoot,
                                 const std::vector<ProjectFile>& files) {
    for (const ProjectFile& file : files) {
        if (std::filesystem::exists(file.path)) {
            return Fail("File already exists: " + file.path.string());
        }
    }

    for (const ProjectFile& file : files) {
        std::string error;
        if (!WriteNewFile(file.path, file.contents, error)) {
            return Fail(error);
        }
    }

    for (const ProjectFile& file : files) {
        if (file.cppFileName.empty()) {
            continue;
        }
        std::string error;
        if (!AppendProjectSystemSource(projectRoot, file.cppFileName, error)) {
            return Fail(error);
        }
    }

    std::string error;
    if (!RegenerateProjectRegister(projectRoot, error)) {
        return Fail(error);
    }

    std::string message = "Created";
    for (const ProjectFile& file : files) {
        message += " " + file.path.string();
    }
    return Succeed(message, "");
}

ScaffoldResult CreateProjectImpl(const ProjectRequest& request) {
    if (request.name.empty()) {
        return Fail("Usage: RTypeEditor new <name> [--path <dir>] [--description <text>]");
    }
    const std::string exeName = ExecutableName(request.name);
    if (exeName.empty()) {
        return Fail("Project name must contain a non-space character");
    }

    std::filesystem::path folder;
    if (request.path.empty()) {
        folder = std::filesystem::current_path() / request.name;
    } else {
        folder = std::filesystem::absolute(request.path);
    }

    const std::filesystem::path scenePath = folder / "test.scene";
    if (std::filesystem::exists(scenePath)) {
        return Fail("Scene already exists: " + scenePath.string());
    }
    if (std::filesystem::exists(folder) && !std::filesystem::is_directory(folder)) {
        return Fail("Path is not a directory: " + folder.string());
    }

    std::filesystem::create_directories(folder / "assets");
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(folder, error);
    if (!error) {
        folder = canonical;
    }

    const std::filesystem::path createdScene = folder / "test.scene";
    std::string writeError;
    if (!WriteEmptyScene(createdScene, folder, request.name, exeName, writeError)) {
        return Fail(writeError);
    }
    if (!std::filesystem::exists(folder / Serialization::kProjectFileName) &&
        !Serialization::SaveProjectFile(folder, Serialization::ProjectFile{})) {
        return Fail("Failed to write " + (folder / Serialization::kProjectFileName).string());
    }

    const std::filesystem::path inputPath = folder / "input.json";
    if (!std::filesystem::exists(inputPath)) {
        if (!WriteNewFile(inputPath, "{}\n", writeError)) {
            return Fail(writeError);
        }
    }

    const std::filesystem::path configDirectory = ConfigDirectory();
    std::filesystem::create_directories(configDirectory);
    const std::filesystem::path settingsPath = configDirectory / "projects.json";

    std::vector<ProjectData> projects;
    if (std::filesystem::exists(settingsPath)) {
        std::ifstream file(settingsPath);
        if (!file) {
            return Fail("Failed to open " + settingsPath.string());
        }
        const std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
        // Same ProjectData map as the Gallery. A failed read must not wipe the list.
        if (glz::read_json(projects, content)) {
            return Fail("Failed to parse " + settingsPath.string());
        }
    }

    bool alreadyListed = false;
    for (const ProjectData& item : projects) {
        std::error_code pathError;
        const std::filesystem::path storedCanonical =
            std::filesystem::weakly_canonical(item.path, pathError);
        if (!pathError && storedCanonical == folder) {
            alreadyListed = true;
            break;
        }
        if (item.path == folder.string()) {
            alreadyListed = true;
            break;
        }
    }

    if (!alreadyListed) {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count();
        ProjectData project;
        project.name = request.name;
        project.description = request.description;
        project.path = folder.string();
        project.lastOpened = std::to_string(now);
        project.engineVersion = "1.0.0";
        projects.push_back(project);

        std::string output;
        if (glz::write_json(projects, output)) {
            return Fail("Failed to write " + settingsPath.string());
        }
        std::ofstream outFile(settingsPath);
        if (!outFile) {
            return Fail("Failed to write " + settingsPath.string());
        }
        outFile << output << '\n';
        if (!outFile) {
            return Fail("Failed to write " + settingsPath.string());
        }
    }

    return Succeed("Created project " + folder.string(), createdScene.string());
}

ScaffoldResult CreateSystemImpl(const std::string& name, const std::string& outputPath) {
    if (!IsIdentifier(name)) {
        return Fail("System name must be a C++ identifier");
    }
    const std::string systemName = WithSuffix(name, "System");
    if (systemName == "System") {
        return Fail("System name conflicts with Engine::System");
    }

    const std::filesystem::path outputRoot = ResolveOutputRoot(outputPath);
    if (std::filesystem::exists(outputRoot) && !std::filesystem::is_directory(outputRoot)) {
        return Fail("Output path is not a directory: " + outputRoot.string());
    }

    const std::optional<std::filesystem::path> cmakeFile = FindEngineCMakeLists(outputRoot);
    if (!cmakeFile.has_value()) {
        return Fail("Could not find Engine CMakeLists.txt from " + outputRoot.string());
    }

    const std::filesystem::path engineRoot = cmakeFile->parent_path();
    if (UsesEngineLayout(outputRoot, engineRoot)) {
        std::vector<EngineFile> files = {
            EngineFile{
                engineRoot / "include" / "Systems" / (systemName + ".hpp"),
                SystemHeader(systemName),
                "INC_FILES",
                "include/Systems/" + systemName + ".hpp",
            },
            EngineFile{
                engineRoot / "src" / "Systems" / (systemName + ".cpp"),
                SystemSource(systemName, "Systems/"),
                "SRC_FILES",
                "src/Systems/" + systemName + ".cpp",
            },
        };
        return WriteEngineFiles(*cmakeFile, files);
    }

    const std::filesystem::path systemsDir = ProjectSystemsDirectory(outputRoot);
    std::vector<ProjectFile> files = {
        ProjectFile{
            systemsDir / (systemName + ".hpp"),
            SystemHeader(systemName),
            "",
        },
        ProjectFile{
            systemsDir / (systemName + ".cpp"),
            SystemSource(systemName, ""),
            systemName + ".cpp",
        },
    };
    return WriteProjectFiles(outputRoot, files);
}

ScaffoldResult CreatePluginImpl(const std::string& name, const std::string& outputPath) {
    if (!IsIdentifier(name)) {
        return Fail("Plugin name must be a C++ identifier");
    }
    const std::string base = WithoutSuffix(name, "Plugin");
    if (!IsIdentifier(base)) {
        return Fail("Plugin name must be a C++ identifier");
    }
    const std::string componentName = base;
    const std::string systemName = WithSuffix(base, "System");
    const std::string pluginName = WithSuffix(base, "Plugin");
    if (systemName == "System") {
        return Fail("System name conflicts with Engine::System");
    }

    const std::filesystem::path outputRoot = ResolveOutputRoot(outputPath);
    if (std::filesystem::exists(outputRoot) && !std::filesystem::is_directory(outputRoot)) {
        return Fail("Output path is not a directory: " + outputRoot.string());
    }

    const std::optional<std::filesystem::path> cmakeFile = FindEngineCMakeLists(outputRoot);
    if (!cmakeFile.has_value()) {
        return Fail("Could not find Engine CMakeLists.txt from " + outputRoot.string());
    }

    const std::filesystem::path engineRoot = cmakeFile->parent_path();
    if (UsesEngineLayout(outputRoot, engineRoot)) {
        std::vector<EngineFile> files = {
            EngineFile{
                engineRoot / "include" / "Components" / (componentName + ".hpp"),
                ComponentHeader(componentName, systemName),
                "INC_FILES",
                "include/Components/" + componentName + ".hpp",
            },
            EngineFile{
                engineRoot / "include" / "Systems" / (systemName + ".hpp"),
                SystemHeader(systemName),
                "INC_FILES",
                "include/Systems/" + systemName + ".hpp",
            },
            EngineFile{
                engineRoot / "src" / "Systems" / (systemName + ".cpp"),
                SystemSource(systemName, "Systems/"),
                "SRC_FILES",
                "src/Systems/" + systemName + ".cpp",
            },
            EngineFile{
                engineRoot / "include" / "Plugins" / (pluginName + ".hpp"),
                PluginHeader(pluginName, componentName, systemName, false),
                "INC_FILES",
                "include/Plugins/" + pluginName + ".hpp",
            },
            EngineFile{
                engineRoot / "src" / "Plugins" / (pluginName + ".cpp"),
                PluginSource(pluginName, componentName, systemName, "Plugins/"),
                "SRC_FILES",
                "src/Plugins/" + pluginName + ".cpp",
            },
        };
        return WriteEngineFiles(*cmakeFile, files);
    }

    const std::filesystem::path systemsDir = ProjectSystemsDirectory(outputRoot);
    std::vector<ProjectFile> files = {
        ProjectFile{
            systemsDir / (componentName + ".hpp"),
            ComponentHeader(componentName, systemName),
            "",
        },
        ProjectFile{
            systemsDir / (systemName + ".hpp"),
            SystemHeader(systemName),
            "",
        },
        ProjectFile{
            systemsDir / (systemName + ".cpp"),
            SystemSource(systemName, ""),
            systemName + ".cpp",
        },
        ProjectFile{
            systemsDir / (pluginName + ".hpp"),
            PluginHeader(pluginName, componentName, systemName, true),
            "",
        },
        ProjectFile{
            systemsDir / (pluginName + ".cpp"),
            PluginSource(pluginName, componentName, systemName, ""),
            pluginName + ".cpp",
        },
    };
    return WriteProjectFiles(outputRoot, files);
}

ScaffoldResult EnsureProjectSceneImpl(const ProjectRequest& request) {
    if (request.name.empty()) {
        return Fail("Project name is empty");
    }
    const std::string exeName = ExecutableName(request.name);
    if (exeName.empty()) {
        return Fail("Project name must contain a non-space character");
    }
    if (request.path.empty()) {
        return Fail("Project path is empty");
    }

    std::filesystem::path folder = std::filesystem::absolute(request.path);
    if (std::filesystem::exists(folder) && !std::filesystem::is_directory(folder)) {
        return Fail("Path is not a directory: " + folder.string());
    }

    std::filesystem::create_directories(folder / "assets");
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(folder, error);
    if (!error) {
        folder = canonical;
    }

    // Open the scene the editor had open last time (project.json), else the
    // first .scene in the project, else test.scene (created if missing).
    const std::filesystem::path scenePath = Serialization::OpenScenePath(folder);
    if (!std::filesystem::exists(scenePath)) {
        std::string writeError;
        if (!WriteEmptyScene(scenePath, folder, request.name, exeName, writeError)) {
            return Fail(writeError);
        }
        log_info("Created scene " + scenePath.string());
    }
    // Older projects have no project.json: write one pointing at that scene.
    if (!std::filesystem::exists(folder / Serialization::kProjectFileName)) {
        Serialization::ProjectFile project;
        project.startupScene =
            std::filesystem::relative(scenePath, folder, error).generic_string();
        if (error || project.startupScene.empty()) {
            project.startupScene = Serialization::kDefaultStartupScene;
        }
        Serialization::SaveProjectFile(folder, project);
    }

    const std::filesystem::path inputPath = folder / "input.json";
    if (!std::filesystem::exists(inputPath)) {
        std::string writeError;
        if (!WriteNewFile(inputPath, "{}\n", writeError)) {
            return Fail(writeError);
        }
    }

    return ScaffoldResult{true, "", scenePath.string()};
}

} // namespace

ScaffoldResult CreateProject(const ProjectRequest& request) {
    try {
        return CreateProjectImpl(request);
    } catch (const std::exception& exception) {
        return Fail(exception.what());
    }
}

ScaffoldResult EnsureProjectScene(const ProjectRequest& request) {
    try {
        return EnsureProjectSceneImpl(request);
    } catch (const std::exception& exception) {
        return Fail(exception.what());
    }
}

ScaffoldResult CreateSystem(const std::string& name, const std::string& enginePath) {
    try {
        return CreateSystemImpl(name, enginePath);
    } catch (const std::exception& exception) {
        return Fail(exception.what());
    }
}

ScaffoldResult CreatePlugin(const std::string& name, const std::string& enginePath) {
    try {
        return CreatePluginImpl(name, enginePath);
    } catch (const std::exception& exception) {
        return Fail(exception.what());
    }
}

} // namespace Editor
