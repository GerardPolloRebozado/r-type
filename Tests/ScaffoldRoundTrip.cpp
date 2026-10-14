#include "GalleryReflect.hpp"
#include "MultiplatformUtils.hpp"
#include "Scaffold.hpp"
#include "SceneSerializer.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <glaze/glaze.hpp>
#include <iostream>
#include <iterator>
#include <string>

#ifndef RTYPE_SOURCE_DIR
#define RTYPE_SOURCE_DIR "."
#endif

static int failures = 0;
#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

static void SetEnvHome(const std::filesystem::path& home) {
    const std::string value = home.string();
#if defined(_WIN32)
    _putenv_s("HOME", value.c_str());
#else
    setenv("HOME", value.c_str(), 1);
#endif
}

static void SetEnvSourceRoot(const std::filesystem::path& root) {
    const std::string value = root.string();
#if defined(_WIN32)
    _putenv_s("RTYPE_SOURCE_DIR", value.c_str());
#else
    setenv("RTYPE_SOURCE_DIR", value.c_str(), 1);
#endif
}

static std::filesystem::path MakeTempDir(const std::string& label) {
    const auto base = std::filesystem::temp_directory_path() / "rtype_scaffold_tests";
    std::filesystem::create_directories(base);
    const auto path = base / label;
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

static void PrepareEngineFixture(const std::filesystem::path& sourceRoot) {
    const std::filesystem::path repoRoot = RTYPE_SOURCE_DIR;
    const std::filesystem::path engineDir = sourceRoot / "Engine";
    std::filesystem::create_directories(engineDir / "include" / "Systems");
    std::filesystem::create_directories(engineDir / "include" / "Components");
    std::filesystem::create_directories(engineDir / "include" / "Plugins");
    std::filesystem::create_directories(engineDir / "src" / "Systems");
    std::filesystem::create_directories(engineDir / "src" / "Plugins");
    std::filesystem::copy_file(repoRoot / "Engine/CMakeLists.txt", engineDir / "CMakeLists.txt",
                               std::filesystem::copy_options::overwrite_existing);
}

static bool CMakeListsContains(const std::filesystem::path& cmakeFile, const std::string& needle) {
    std::ifstream in(cmakeFile);
    if (!in) {
        return false;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (line.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static void TestCreateProject(const std::filesystem::path& home) {
    SetEnvHome(home);

    const std::filesystem::path projectDir = home / "MyProject";
    Editor::ProjectRequest request;
    request.name = "My Project";
    request.description = "scaffold test";
    request.path = projectDir.string();

    Editor::ScaffoldResult created = Editor::CreateProject(request);
    CHECK(created.ok);
    CHECK(std::filesystem::exists(projectDir / "test.scene"));
    CHECK(std::filesystem::exists(projectDir / "assets"));
    CHECK(std::filesystem::exists(projectDir / "input.json"));
    CHECK(!created.scenePath.empty());

    Engine::Registry registry;
    Serialization::SceneConfig config;
    Serialization::SceneSerializer serializer;
    CHECK(serializer.Deserialize(created.scenePath, registry, config));
    CHECK(config.windowTitle == "My Project");
    CHECK(config.m_BuildExecutableName == "MyProject");
    CHECK(config.windowWidth == 1280);
    CHECK(config.windowHeight == 720);
    CHECK(config.targetFPS == 60);

    const std::filesystem::path settingsPath = Editor::ConfigDirectory() / "projects.json";
    CHECK(std::filesystem::exists(settingsPath));
    std::ifstream settingsFile(settingsPath);
    const std::string settingsContent((std::istreambuf_iterator<char>(settingsFile)),
                                      std::istreambuf_iterator<char>());
    std::vector<Editor::ProjectData> listed;
    CHECK(!glz::read_json(listed, settingsContent));
    bool found = false;
    for (const Editor::ProjectData& project : listed) {
        if (project.name == "My Project") {
            found = true;
            CHECK(project.description == "scaffold test");
            CHECK(project.path == projectDir.string() ||
                  std::filesystem::weakly_canonical(project.path) ==
                      std::filesystem::weakly_canonical(projectDir));
            CHECK(project.engineVersion == "1.0.0");
        }
    }
    CHECK(found);

    Editor::ScaffoldResult duplicate = Editor::CreateProject(request);
    CHECK(!duplicate.ok);
}

static void TestCreateSystem(const std::filesystem::path& sourceRoot) {
    SetEnvSourceRoot(sourceRoot);
    PrepareEngineFixture(sourceRoot);

    const std::filesystem::path engineDir = sourceRoot / "Engine";
    Editor::ScaffoldResult created = Editor::CreateSystem("RoundTrip");
    CHECK(created.ok);
    CHECK(std::filesystem::exists(engineDir / "include/Systems/RoundTripSystem.hpp"));
    CHECK(std::filesystem::exists(engineDir / "src/Systems/RoundTripSystem.cpp"));
    CHECK(CMakeListsContains(engineDir / "CMakeLists.txt", "src/Systems/RoundTripSystem.cpp"));
    CHECK(CMakeListsContains(engineDir / "CMakeLists.txt", "include/Systems/RoundTripSystem.hpp"));

    Editor::ScaffoldResult duplicate = Editor::CreateSystem("RoundTrip");
    CHECK(!duplicate.ok);

    Editor::ScaffoldResult suffixed = Editor::CreateSystem("RoundTripSystem");
    CHECK(!suffixed.ok);

    Editor::ScaffoldResult invalid = Editor::CreateSystem("9Bad");
    CHECK(!invalid.ok);
}

static void TestCreatePlugin(const std::filesystem::path& sourceRoot) {
    SetEnvSourceRoot(sourceRoot);
    PrepareEngineFixture(sourceRoot);

    const std::filesystem::path engineDir = sourceRoot / "Engine";
    Editor::ScaffoldResult created = Editor::CreatePlugin("RoundTrip");
    CHECK(created.ok);
    CHECK(std::filesystem::exists(engineDir / "include/Components/RoundTrip.hpp"));
    CHECK(std::filesystem::exists(engineDir / "include/Systems/RoundTripSystem.hpp"));
    CHECK(std::filesystem::exists(engineDir / "src/Systems/RoundTripSystem.cpp"));
    CHECK(std::filesystem::exists(engineDir / "include/Plugins/RoundTripPlugin.hpp"));
    CHECK(std::filesystem::exists(engineDir / "src/Plugins/RoundTripPlugin.cpp"));
    CHECK(CMakeListsContains(engineDir / "CMakeLists.txt", "src/Plugins/RoundTripPlugin.cpp"));

    std::ifstream pluginSource(engineDir / "src/Plugins/RoundTripPlugin.cpp");
    std::string pluginText((std::istreambuf_iterator<char>(pluginSource)),
                           std::istreambuf_iterator<char>());
    CHECK(pluginText.find("RegisterComponent<RoundTrip>") != std::string::npos);
    CHECK(pluginText.find("RegisterSystem<RoundTripSystem>") != std::string::npos);

    Editor::ScaffoldResult duplicate = Editor::CreatePlugin("RoundTrip");
    CHECK(!duplicate.ok);
}

static void TestProjectListJson() {
    Editor::ProjectData written;
    written.name = "Json Round Trip";
    written.description = "gallery list";
    written.lastOpened = "10";
    written.path = "/tmp/gallery-json";
    written.engineVersion = "1.0.0";

    std::vector<Editor::ProjectData> projects = {written};
    std::string json;
    CHECK(!glz::write_json(projects, json));

    std::vector<Editor::ProjectData> loaded;
    CHECK(!glz::read_json(loaded, json));
    CHECK(loaded.size() == 1);
    CHECK(loaded[0].name == written.name);
    CHECK(loaded[0].description == written.description);
    CHECK(loaded[0].lastOpened == written.lastOpened);
    CHECK(loaded[0].path == written.path);
    CHECK(loaded[0].engineVersion == written.engineVersion);
}

int main() {
    const std::filesystem::path home = MakeTempDir("home");
    const std::filesystem::path systemRoot = MakeTempDir("system_root");
    const std::filesystem::path pluginRoot = MakeTempDir("plugin_root");

    TestProjectListJson();
    TestCreateProject(home);
    TestCreateSystem(systemRoot);
    TestCreatePlugin(pluginRoot);

    std::filesystem::remove_all(home);
    std::filesystem::remove_all(systemRoot);
    std::filesystem::remove_all(pluginRoot);

    if (failures == 0) {
        std::cout << "ALL CHECKS PASSED\n";
        return 0;
    }
    std::cerr << failures << " CHECKS FAILED\n";
    return 1;
}
