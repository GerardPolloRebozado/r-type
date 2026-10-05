/**
 * @file Logger.hpp
 * @brief Compile-time filtered logging macros for the Engine.
 *
 * Provides @ref log_error, @ref log_debug and @ref log_info macros that print
 * tagged messages to the terminal. Macros below the configured @c ENGINE_LOG_LEVEL
 * are stripped at compile time and produce no code.
 */

#ifndef ENGINE_LOGGING_LOGGER_HPP_
#define ENGINE_LOGGING_LOGGER_HPP_

#include "Logging/LogConfig.hpp"
#include <iostream>
#include <string>

namespace Engine {

enum class LogLevel { Error, Debug, Info };

/**
 * @brief Single write path for all log levels (terminal today).
 *
 * A later ring buffer will be pushed from here so Editor ConsolePanel can
 * display engine logs without changing call sites.
 */
inline void emit(LogLevel level, const std::string& msg)
{
    switch (level) {
    case LogLevel::Error:
        std::cerr << "[ERROR] " << msg << std::endl;
        break;
    case LogLevel::Debug:
        std::cout << "[DEBUG] " << msg << std::endl;
        break;
    case LogLevel::Info:
        std::cout << "[INFO] " << msg << std::endl;
        break;
    }
}

inline void log_error(const std::string& msg) { emit(LogLevel::Error, msg); }

inline void log_debug(const std::string& msg) { emit(LogLevel::Debug, msg); }

inline void log_info(const std::string& msg) { emit(LogLevel::Info, msg); }

} // namespace Engine

#if ENGINE_LOG_LEVEL >= ENGINE_LOG_ERROR
#define log_error(msg) Engine::log_error((msg))
#else
#define log_error(msg) ((void)0)
#endif

#if ENGINE_LOG_LEVEL >= ENGINE_LOG_DEBUG
#define log_debug(msg) Engine::log_debug((msg))
#else
#define log_debug(msg) ((void)0)
#endif

#if ENGINE_LOG_LEVEL >= ENGINE_LOG_INFO
#define log_info(msg) Engine::log_info((msg))
#else
#define log_info(msg) ((void)0)
#endif

#endif // ENGINE_LOGGING_LOGGER_HPP_
