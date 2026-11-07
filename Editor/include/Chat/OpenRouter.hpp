#ifndef EDITOR_CHAT_OPENROUTER_HPP_
#define EDITOR_CHAT_OPENROUTER_HPP_

#include <string>
#include <vector>

namespace Editor {

/**
 * @brief One tool call from an OpenAI-compatible chat completion.
 */
struct ChatToolCall {
    std::string id;
    std::string name;
    std::string arguments;
};

/**
 * @brief Text and tool calls from one completion, or an error.
 */
struct ChatCompletion {
    std::string content;
    std::vector<ChatToolCall> toolCalls;
    std::string error;
};

/**
 * @brief One HTTPS chat completion. The API key is not placed on the command line.
 */
struct ChatRequest {
    std::string apiKey;
    std::string url;
    std::string model;
    std::string messagesJson;
    std::string toolsJson;
};

ChatCompletion CompleteChat(const ChatRequest& request);

/** @brief Turns the author tool catalog into OpenAI function tools. */
std::string OpenAiToolsJson(const char* catalogJson);

std::string SystemMessageJson(const std::string& text);
std::string UserMessageJson(const std::string& text);
std::string AssistantMessageJson(const ChatCompletion& completion);
std::string ToolMessageJson(const std::string& id, const std::string& content);

} // namespace Editor

#endif // EDITOR_CHAT_OPENROUTER_HPP_
