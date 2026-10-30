/**
 * @file Logger.hpp
 * @brief Compile-time filtered logging macros for the Engine.
 *
 * Provides @ref log_error, @ref log_user_error, @ref log_debug and @ref log_info.
 * Every emitted line is queued for the terminal worker. Only @ref log_user_error
 * is stored in @ref LogBuffer for the editor Console. Macros below the configured
 * @c ENGINE_LOG_LEVEL are stripped at compile time and produce no code.
 */

#ifndef ENGINE_LOGGING_LOGGER_HPP_
#define ENGINE_LOGGING_LOGGER_HPP_

#include "Logging/LogBuffer.hpp"
#include "Logging/LogConfig.hpp"
#include "Logging/LogWriter.hpp"
#include <string>
#include <utility>

namespace Engine {

/**
 * @brief Single write path for all log levels.
 *
 * User lines are copied into @ref LogBuffer, then every line is moved to the
 * terminal queue. The caller does not touch @c std::cout or @c std::cerr.
 * Call sites stay on the @c log_* macros.
 *
 * @param level Severity of the line.
 * @param audience Engine lines stay on the terminal. User lines also reach the Console.
 * @param msg Raw message, without the terminal tag.
 */
inline void emit(LogLevel level, LogAudience audience, std::string msg) {
    if (audience == LogAudience::User) {
        GetLogBuffer().Push(level, msg);
    }
    EnqueueLog(level, std::move(msg));
}

inline void log_error(std::string msg) {
    emit(LogLevel::Error, LogAudience::Engine, std::move(msg));
}

/**
 * @brief Records one user-caused error.
 *
 * The line is shown in the editor Console and written to the terminal.
 * Use this when the failure comes from content or input the user controls.
 *
 * @param msg Raw message, without the terminal tag.
 */
inline void log_user_error(std::string msg) {
    emit(LogLevel::Error, LogAudience::User, std::move(msg));
}

inline void log_debug(std::string msg) {
    emit(LogLevel::Debug, LogAudience::Engine, std::move(msg));
}

inline void log_info(std::string msg) {
    emit(LogLevel::Info, LogAudience::Engine, std::move(msg));
}

} // namespace Engine

#if ENGINE_LOG_LEVEL >= ENGINE_LOG_ERROR
#define log_error(msg) Engine::log_error((msg))
#define log_user_error(msg) Engine::log_user_error((msg))
#else
#define log_error(msg) ((void)0)
#define log_user_error(msg) ((void)0)
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
