#include "Chat/OpenRouter.hpp"
#include <glaze/glaze.hpp>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <stdio.h>
#else
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace Editor {
namespace {

constexpr int kMaxSeconds = 120;

std::string JsonObject(const glz::generic& value)
{
    std::string json;
    if (glz::write_json(value, json)) {
        return {};
    }
    return json;
}

std::string Redact(std::string text, const std::string& secret)
{
    std::size_t found = 0;

    if (secret.empty()) {
        return text;
    }
    while ((found = text.find(secret, found)) != std::string::npos) {
        text.replace(found, secret.size(), "[redacted]");
        found += std::strlen("[redacted]");
    }
    return text;
}

bool KeyIsSafe(const std::string& key)
{
    return key.find('"') == std::string::npos && key.find('\\') == std::string::npos &&
           key.find('\n') == std::string::npos && key.find('\r') == std::string::npos;
}

std::string WriteTemp(const std::string& bytes, std::string& error)
{
    std::string pattern = std::filesystem::temp_directory_path().string() + "/rtype-chat-XXXXXX";
    std::vector<char> path(pattern.begin(), pattern.end());
    int fd = 0;

    path.push_back('\0');
#ifdef _WIN32
    char tmp[L_tmpnam];
    std::ofstream out;
    if (std::tmpnam(tmp) == nullptr) {
        error = "could not create a temp file for curl";
        return {};
    }
    out.open(tmp, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "could not create a temp file for curl";
        return {};
    }
    out << bytes;
    out.close();
    return tmp;
#else
    fd = mkstemp(path.data());
    if (fd < 0) {
        error = "could not create a temp file for curl";
        return {};
    }
    if (!bytes.empty() && write(fd, bytes.data(), bytes.size()) != static_cast<ssize_t>(bytes.size())) {
        error = "could not write the curl body";
        close(fd);
        unlink(path.data());
        return {};
    }
    close(fd);
    return path.data();
#endif
}

#ifndef _WIN32
bool ReadPipes(int outFd, int errFd, std::string& out, std::string& err)
{
    bool outOpen = true;
    bool errOpen = true;
    char buffer[4096];

    while (outOpen || errOpen) {
        pollfd fds[2];
        int count = 0;
        int ready = 0;

        if (outOpen) {
            fds[count].fd = outFd;
            fds[count].events = POLLIN;
            fds[count].revents = 0;
            ++count;
        }
        if (errOpen) {
            fds[count].fd = errFd;
            fds[count].events = POLLIN;
            fds[count].revents = 0;
            ++count;
        }
        ready = poll(fds, static_cast<nfds_t>(count), -1);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        for (int index = 0; index < count; ++index) {
            ssize_t got = 0;
            if ((fds[index].revents & (POLLIN | POLLHUP | POLLERR)) == 0) {
                continue;
            }
            got = read(fds[index].fd, buffer, sizeof(buffer));
            if (got < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return false;
            }
            if (got > 0) {
                if (fds[index].fd == outFd) {
                    out.append(buffer, static_cast<std::size_t>(got));
                } else {
                    err.append(buffer, static_cast<std::size_t>(got));
                }
            }
            if (got == 0 || (fds[index].revents & (POLLHUP | POLLERR))) {
                if (fds[index].fd == outFd) {
                    outOpen = false;
                } else {
                    errOpen = false;
                }
            }
        }
    }
    return true;
}

std::string RunCurl(const std::string& key, const std::string& url, const std::string& bodyPath,
                     std::string& error)
{
    int outPipe[2] = {-1, -1};
    int errPipe[2] = {-1, -1};
    int cfgPipe[2] = {-1, -1};
    posix_spawn_file_actions_t actions;
    pid_t pid = 0;
    std::string config = "header = \"Authorization: Bearer " + key + "\"\n";
    std::string dataArg = "@" + bodyPath;
    std::string seconds = std::to_string(kMaxSeconds);
    std::string out;
    std::string err;
    int status = 0;
    char* argv[] = {const_cast<char*>("curl"),
                    const_cast<char*>("-sS"),
                    const_cast<char*>("--fail-with-body"),
                    const_cast<char*>("--max-time"),
                    seconds.data(),
                    const_cast<char*>("-K"),
                    const_cast<char*>("/dev/fd/3"),
                    const_cast<char*>("-H"),
                    const_cast<char*>("Content-Type: application/json"),
                    const_cast<char*>("--data-binary"),
                    dataArg.data(),
                    const_cast<char*>(url.c_str()),
                    nullptr};

    if (pipe(outPipe) != 0 || pipe(errPipe) != 0 || pipe(cfgPipe) != 0) {
        error = "could not create pipes for curl";
        return {};
    }
    if (write(cfgPipe[1], config.data(), config.size()) != static_cast<ssize_t>(config.size())) {
        error = "could not pass the API key to curl";
        close(outPipe[0]);
        close(outPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);
        close(cfgPipe[0]);
        close(cfgPipe[1]);
        return {};
    }
    close(cfgPipe[1]);
    cfgPipe[1] = -1;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, outPipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, errPipe[1], STDERR_FILENO);
    posix_spawn_file_actions_adddup2(&actions, cfgPipe[0], 3);
    posix_spawn_file_actions_addclose(&actions, outPipe[0]);
    posix_spawn_file_actions_addclose(&actions, errPipe[0]);
    posix_spawn_file_actions_addclose(&actions, cfgPipe[0]);
    if (posix_spawnp(&pid, "curl", &actions, nullptr, argv, environ) != 0) {
        error = "curl was not found";
        posix_spawn_file_actions_destroy(&actions);
        close(outPipe[0]);
        close(outPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);
        close(cfgPipe[0]);
        return {};
    }
    posix_spawn_file_actions_destroy(&actions);
    close(outPipe[1]);
    close(errPipe[1]);
    close(cfgPipe[0]);
    if (!ReadPipes(outPipe[0], errPipe[0], out, err)) {
        error = "failed while reading curl";
    }
    close(outPipe[0]);
    close(errPipe[0]);
    waitpid(pid, &status, 0);
    if (!error.empty()) {
        return {};
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        error = err.empty() ? out : err;
        if (error.empty()) {
            error = "curl failed";
        }
        return out;
    }
    return out;
}
#else
std::string RunCurl(const std::string& key, const std::string& url, const std::string& bodyPath,
                     std::string& error)
{
    std::string configPath;
    std::string config = "header = \"Authorization: Bearer " + key + "\"\n";
    std::string command;
    FILE* pipe = nullptr;
    std::string out;
    char buffer[4096];

    configPath = WriteTemp(config, error);
    if (configPath.empty()) {
        return {};
    }
    command = "curl.exe -sS --fail-with-body --max-time " + std::to_string(kMaxSeconds) + " -K \"" +
              configPath + "\" -H \"Content-Type: application/json\" --data-binary \"@" + bodyPath +
              "\" \"" + url + "\"";
    pipe = _popen(command.c_str(), "r");
    if (pipe == nullptr) {
        error = "curl was not found";
        std::filesystem::remove(configPath);
        return {};
    }
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        out += buffer;
    }
    if (_pclose(pipe) != 0 && out.empty()) {
        error = "curl failed";
    }
    std::filesystem::remove(configPath);
    return out;
}
#endif

ChatCompletion ParseCompletion(const std::string& body)
{
    ChatCompletion completion;
    glz::generic root;
    const glz::generic::object_t* obj = nullptr;
    const glz::generic* message = nullptr;

    if (glz::read_json(root, body) || !root.is_object()) {
        completion.error = body.empty() ? "empty response from the model" : body;
        return completion;
    }
    obj = &root.get_object();
    if (obj->find("error") != obj->end()) {
        const glz::generic& err = obj->at("error");
        std::string message;
        std::string raw;

        if (err.is_object() && err.get_object().find("message") != err.get_object().end() &&
            err.get_object().at("message").is_string()) {
            message = err.get_object().at("message").get_string();
        }
        if (err.is_object() && err.get_object().find("metadata") != err.get_object().end() &&
            err.get_object().at("metadata").is_object()) {
            const glz::generic::object_t& meta = err.get_object().at("metadata").get_object();
            if (meta.find("raw") != meta.end() && meta.at("raw").is_string()) {
                raw = meta.at("raw").get_string();
            } else if (meta.find("raw") != meta.end()) {
                raw = JsonObject(meta.at("raw"));
            }
        }
        if (!raw.empty() && raw != message) {
            if (!message.empty()) {
                message += ": ";
            }
            message += raw;
        }
        completion.error = message.empty() ? JsonObject(err) : message;
        return completion;
    }
    if (obj->find("choices") == obj->end() || !obj->at("choices").is_array() ||
        obj->at("choices").get_array().empty()) {
        completion.error = "model response has no choices";
        return completion;
    }
    const glz::generic& choice = obj->at("choices").get_array().front();
    if (!choice.is_object() || choice.get_object().find("message") == choice.get_object().end()) {
        completion.error = "model response has no message";
        return completion;
    }
    message = &choice.get_object().at("message");
    if (!message->is_object()) {
        completion.error = "model response has no message";
        return completion;
    }
    if (message->get_object().find("content") != message->get_object().end() &&
        message->get_object().at("content").is_string()) {
        completion.content = message->get_object().at("content").get_string();
    }
    if (message->get_object().find("tool_calls") != message->get_object().end() &&
        message->get_object().at("tool_calls").is_array()) {
        int callIndex = 0;
        for (const glz::generic& call : message->get_object().at("tool_calls").get_array()) {
            ChatToolCall tool;
            const glz::generic::object_t* callObj = nullptr;
            const glz::generic::object_t* fn = nullptr;

            if (!call.is_object()) {
                continue;
            }
            callObj = &call.get_object();
            if (callObj->find("id") != callObj->end() && callObj->at("id").is_string()) {
                tool.id = callObj->at("id").get_string();
            }
            if (callObj->find("function") == callObj->end() || !callObj->at("function").is_object()) {
                continue;
            }
            fn = &callObj->at("function").get_object();
            if (fn->find("name") != fn->end() && fn->at("name").is_string()) {
                tool.name = fn->at("name").get_string();
            }
            if (fn->find("arguments") != fn->end()) {
                if (fn->at("arguments").is_string()) {
                    tool.arguments = fn->at("arguments").get_string();
                } else {
                    tool.arguments = JsonObject(fn->at("arguments"));
                }
            }
            if (tool.arguments.empty()) {
                tool.arguments = "{}";
            }
            if (tool.id.empty()) {
                tool.id = "call-" + std::to_string(callIndex);
            }
            ++callIndex;
            completion.toolCalls.push_back(tool);
        }
    }
    return completion;
}

void SanitizeSchema(glz::generic& schema)
{
    glz::generic::object_t* obj = nullptr;
    std::string kind;

    if (!schema.is_object()) {
        return;
    }
    obj = &schema.get_object();
    obj->erase("additionalProperties");
    if (obj->find("type") != obj->end() && obj->at("type").is_string()) {
        kind = obj->at("type").get_string();
    }
    if (kind == "integer") {
        obj->at("type") = std::string("number");
        kind = "number";
    }
    if (kind == "object" && obj->find("properties") == obj->end()) {
        (*obj)["properties"] = glz::generic::object_t{};
    }
    if (kind == "array" && obj->find("items") == obj->end()) {
        glz::generic items = glz::generic::object_t{};
        items.get_object()["type"] = std::string("object");
        items.get_object()["properties"] = glz::generic::object_t{};
        (*obj)["items"] = std::move(items);
    }
    if (obj->find("properties") != obj->end() && obj->at("properties").is_object()) {
        for (auto& property : obj->at("properties").get_object()) {
            SanitizeSchema(property.second);
        }
    }
    if (obj->find("items") != obj->end()) {
        SanitizeSchema(obj->at("items"));
    }
}

bool RateLimited(const std::string& message)
{
    std::string lower = message;
    std::size_t index = 0;

    for (index = 0; index < lower.size(); ++index) {
        lower[index] = static_cast<char>(std::tolower(static_cast<unsigned char>(lower[index])));
    }
    return lower.find("rate-limited") != std::string::npos ||
           lower.find("rate limit") != std::string::npos || lower.find("429") != std::string::npos;
}

} // namespace

std::string OpenAiToolsJson(const char* catalogJson)
{
    glz::generic catalog;
    glz::generic tools = glz::generic::array_t{};
    std::string json;

    if (catalogJson == nullptr || glz::read_json(catalog, std::string(catalogJson)) ||
        !catalog.is_array()) {
        return "[]";
    }
    for (const glz::generic& item : catalog.get_array()) {
        glz::generic tool = glz::generic::object_t{};
        glz::generic function = glz::generic::object_t{};

        if (!item.is_object()) {
            continue;
        }
        function.get_object()["name"] = item.get_object().at("name");
        function.get_object()["description"] = item.get_object().at("description");
        if (item.get_object().find("inputSchema") != item.get_object().end()) {
            glz::generic parameters = item.get_object().at("inputSchema");
            SanitizeSchema(parameters);
            function.get_object()["parameters"] = std::move(parameters);
        } else {
            function.get_object()["parameters"] = glz::generic::object_t{};
        }
        tool.get_object()["type"] = std::string("function");
        tool.get_object()["function"] = std::move(function);
        tools.get_array().push_back(std::move(tool));
    }
    if (glz::write_json(tools, json)) {
        return "[]";
    }
    return json;
}

std::string SystemMessageJson(const std::string& text)
{
    glz::generic message = glz::generic::object_t{};
    message.get_object()["role"] = std::string("system");
    message.get_object()["content"] = text;
    return JsonObject(message);
}

std::string UserMessageJson(const std::string& text)
{
    glz::generic message = glz::generic::object_t{};
    message.get_object()["role"] = std::string("user");
    message.get_object()["content"] = text;
    return JsonObject(message);
}

std::string AssistantMessageJson(const ChatCompletion& completion)
{
    glz::generic message = glz::generic::object_t{};
    glz::generic calls = glz::generic::array_t{};

    message.get_object()["role"] = std::string("assistant");
    message.get_object()["content"] = completion.content;
    for (const ChatToolCall& call : completion.toolCalls) {
        glz::generic tool = glz::generic::object_t{};
        glz::generic function = glz::generic::object_t{};
        function.get_object()["name"] = call.name;
        function.get_object()["arguments"] = call.arguments;
        tool.get_object()["id"] = call.id;
        tool.get_object()["type"] = std::string("function");
        tool.get_object()["function"] = std::move(function);
        calls.get_array().push_back(std::move(tool));
    }
    if (!completion.toolCalls.empty()) {
        message.get_object()["tool_calls"] = std::move(calls);
    }
    return JsonObject(message);
}

std::string ToolMessageJson(const std::string& id, const std::string& content)
{
    glz::generic message = glz::generic::object_t{};
    message.get_object()["role"] = std::string("tool");
    message.get_object()["tool_call_id"] = id;
    message.get_object()["content"] = content;
    return JsonObject(message);
}

ChatCompletion CompleteChat(const ChatRequest& request)
{
    ChatCompletion completion;
    glz::generic body = glz::generic::object_t{};
    glz::generic messages;
    glz::generic tools;
    std::string bodyJson;
    std::string bodyPath;
    std::string response;
    std::string error;

    if (request.apiKey.empty()) {
        completion.error = "API key is empty. Open the gear and paste a key.";
        return completion;
    }
    if (!KeyIsSafe(request.apiKey)) {
        completion.error = "API key contains a quote or a newline.";
        return completion;
    }
    if (request.url.empty() || request.model.empty()) {
        completion.error = "model and endpoint are required";
        return completion;
    }
    if (glz::read_json(messages, request.messagesJson) || !messages.is_array()) {
        completion.error = "could not build the chat request";
        return completion;
    }
    body.get_object()["model"] = request.model;
    body.get_object()["messages"] = std::move(messages);
    if (!request.toolsJson.empty() && !glz::read_json(tools, request.toolsJson) && tools.is_array() &&
        !tools.get_array().empty()) {
        body.get_object()["tools"] = std::move(tools);
        body.get_object()["tool_choice"] = std::string("auto");
    }
    bodyJson = JsonObject(body);
    if (bodyJson.empty()) {
        completion.error = "could not encode the chat request";
        return completion;
    }
    bodyPath = WriteTemp(bodyJson, error);
    if (bodyPath.empty()) {
        completion.error = error;
        return completion;
    }
    for (int attempt = 0; attempt < 3; ++attempt) {
        std::string reason;

        error.clear();
        response = RunCurl(request.apiKey, request.url, bodyPath, error);
        response = Redact(response, request.apiKey);
        error = Redact(error, request.apiKey);
        if (!error.empty() && response.empty()) {
            completion.error = error;
            completion.content.clear();
            completion.toolCalls.clear();
        } else {
            completion = ParseCompletion(response);
            completion.error = Redact(completion.error, request.apiKey);
            if (completion.error.empty() && !error.empty()) {
                completion.error = error;
            }
        }
        reason = completion.error;
        if (reason.empty() || attempt == 2 || !RateLimited(reason)) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2 * (attempt + 1)));
    }
    std::filesystem::remove(bodyPath);
    return completion;
}

} // namespace Editor
