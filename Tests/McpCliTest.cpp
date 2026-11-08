#include "Author/Session.hpp"
#include "Components/Name.hpp"
#include "SceneSerializer.hpp"
#include "UI/UiComponent.hpp"
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

static int failures = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                              \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

namespace {

std::filesystem::path EditorBinary(const char* argv0)
{
    std::filesystem::path self = std::filesystem::absolute(argv0);
    std::filesystem::path editor = self.parent_path() / "RTypeEditor";
#ifdef _WIN32
    if (!std::filesystem::exists(editor)) {
        editor = self.parent_path() / "RTypeEditor.exe";
    }
#endif
    return editor;
}

#ifndef _WIN32

struct Child {
    pid_t pid = -1;
    int input = -1;
    int output = -1;
};

bool Spawn(const std::filesystem::path& editor, const char* flag, Child& child)
{
    int toChild[2] = {-1, -1};
    int fromChild[2] = {-1, -1};
    pid_t pid = -1;

    if (pipe(toChild) != 0 || pipe(fromChild) != 0) {
        return false;
    }
    pid = fork();
    if (pid < 0) {
        return false;
    }
    if (pid == 0) {
        dup2(toChild[0], STDIN_FILENO);
        dup2(fromChild[1], STDOUT_FILENO);
        close(toChild[0]);
        close(toChild[1]);
        close(fromChild[0]);
        close(fromChild[1]);
        execl(editor.c_str(), editor.c_str(), flag, static_cast<char*>(nullptr));
        _exit(127);
    }
    close(toChild[0]);
    close(fromChild[1]);
    child.pid = pid;
    child.input = toChild[1];
    child.output = fromChild[0];
    return true;
}

bool WriteAll(int fd, const std::string& bytes)
{
    std::size_t sent = 0;

    while (sent < bytes.size()) {
        const ssize_t wrote = write(fd, bytes.data() + sent, bytes.size() - sent);
        if (wrote <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(wrote);
    }
    return true;
}

std::string ReadAll(int fd)
{
    std::string data;
    char buffer[4096];
    ssize_t count = 0;

    while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
        data.append(buffer, static_cast<std::size_t>(count));
    }
    return data;
}

std::string Exchange(const std::filesystem::path& editor, const char* flag,
                     const std::vector<std::string>& lines)
{
    Child child;
    std::string output;
    std::thread reader;
    int status = 0;

    if (!Spawn(editor, flag, child)) {
        return {};
    }
    reader = std::thread([&output, &child] { output = ReadAll(child.output); });
    for (const std::string& line : lines) {
        if (!WriteAll(child.input, line + "\n")) {
            break;
        }
    }
    close(child.input);
    reader.join();
    close(child.output);
    waitpid(child.pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) == 127) {
        return {};
    }
    return output;
}

#endif

bool SceneHasMenu(const std::filesystem::path& scene, const std::filesystem::path& project)
{
    Engine::Registry registry;
    Serialization::SceneSerializer serializer;
    Serialization::SceneConfig config;
    std::size_t index = 0;

    Author::RegisterAuthorComponents(registry);
    config.m_ProjectFolder = project.string();
    if (!serializer.Deserialize(scene.string(), registry, config)) {
        return false;
    }
    if (!registry.IsComponentRegistered<Engine::UiComponent>()) {
        return false;
    }
    for (index = 1; index < registry.GetEntityCount(); ++index) {
        if (!registry.IsEntityAlive(index) ||
            !registry.HasComponent<Engine::UiComponent>(index)) {
            continue;
        }
        if (registry.RequireComponent<Engine::UiComponent>(index).templateName == "menu") {
            return true;
        }
    }
    return false;
}

} // namespace

int main(int argc, char** argv)
{
#ifndef _WIN32
    const std::filesystem::path editor = EditorBinary(argc > 0 ? argv[0] : "McpCliTest");
    const std::filesystem::path source(RTYPE_SOURCE_DIR);
    const std::filesystem::path project =
        std::filesystem::temp_directory_path() / "rtype-mcp-cli-test";
    std::error_code error;
    std::string listed;
    std::string edited;
    std::string alias;
    const std::string call =
        std::string("{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"tools/call\",\"params\":{"
                    "\"name\":\"add_ui_screen\",\"arguments\":{\"projectFolder\":\"") +
        project.string() +
        "\",\"template\":\"menu\",\"name\":\"MainMenu\",\"startPage\":true}}}";

    std::filesystem::remove_all(project, error);
    std::filesystem::create_directories(project, error);
    std::filesystem::copy_file(source / "Games/Menu/test.scene", project / "test.scene", error);
    std::filesystem::copy_file(source / "Games/Menu/project.json", project / "project.json", error);
    CHECK(std::filesystem::exists(editor));
    CHECK(std::filesystem::exists(project / "test.scene"));

    listed = Exchange(editor, "mcp",
                      {"{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}",
                       "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}",
                       "{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"tools/list\"}"});
    CHECK(listed.find("2024-11-05") != std::string::npos);
    CHECK(listed.find("create_entity") != std::string::npos);
    CHECK(listed.find("add_ui_widget") != std::string::npos);
    CHECK(listed.find("[INFO]") == std::string::npos);
    CHECK(listed.find("[DEBUG]") == std::string::npos);

    edited = Exchange(editor, "mcp",
                      {"{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}",
                       call,
                       "{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"nope\"}"});
    CHECK(edited.find("mainmenu") != std::string::npos);
    CHECK(edited.find("-32601") != std::string::npos);
    CHECK(edited.find("[INFO]") == std::string::npos);
    CHECK(SceneHasMenu(project / "test.scene", project));

    alias = Exchange(editor, "--mcp",
                     {"{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}"});
    CHECK(alias.find("2024-11-05") != std::string::npos);

    std::filesystem::remove_all(project, error);
#else
    (void)argc;
    (void)argv;
    std::cerr << "McpCliTest: POSIX pipes required\n";
    return 1;
#endif

    if (failures == 0) {
        std::cout << "McpCliTest: all checks passed\n";
        return 0;
    }
    std::cerr << failures << " checks failed\n";
    return 1;
}
