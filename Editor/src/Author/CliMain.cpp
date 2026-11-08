#include "Author/Commands.hpp"
#include "Author/Protocol.hpp"
#include "Logging/LogWriter.hpp"
#include <glaze/glaze.hpp>
#include <iostream>
#include <iterator>
#include <string>

int main()
{
    std::string input((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
    Author::AuthorRequest request;
    glz::generic response;
    std::string json;
    constexpr glz::opts options{.error_on_unknown_keys = false};

    if (input.empty()) {
        response = Author::RunCommand(Author::AuthorRequest{.command = ""});
        response.get_object()["ok"] = false;
        response.get_object()["error"] = "expected one JSON object on stdin";
        response.get_object()["apiVersion"] = Author::kApiVersion;
    } else if (const auto error = glz::read<options>(request, input)) {
        response = glz::generic::object_t{};
        response.get_object()["ok"] = false;
        response.get_object()["apiVersion"] = Author::kApiVersion;
        response.get_object()["error"] = "invalid JSON: " + glz::format_error(error, input);
    } else {
        try {
            response = Author::RunCommand(request);
        } catch (const std::exception& exception) {
            response = glz::generic::object_t{};
            response.get_object()["ok"] = false;
            response.get_object()["apiVersion"] = Author::kApiVersion;
            response.get_object()["error"] = exception.what();
        }
    }
    Engine::ShutdownLogging();
    if (glz::write_json(response, json)) {
        std::cout << "{\"apiVersion\":1,\"ok\":false,\"error\":\"failed to encode response\"}\n";
        return 1;
    }
    std::cout << json << '\n';
    return response.is_object() && response.get_object().contains("ok") &&
                   response.get_object()["ok"].is_boolean() &&
                   response.get_object()["ok"].get<bool>()
               ? 0
               : 1;
}
