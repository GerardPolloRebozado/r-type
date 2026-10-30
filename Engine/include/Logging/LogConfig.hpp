/**
 * @file LogConfig.hpp
 * @brief Compile-time log level constants for the Engine logger.
 *
 * Log verbosity is controlled at build time via the CMake cache variable
 * @c ENGINE_LOG_LEVEL (passed as a compile definition on the @c Engine target).
 */

#ifndef ENGINE_LOGGING_LOG_CONFIG_HPP_
#define ENGINE_LOGGING_LOG_CONFIG_HPP_

/** @brief Minimum level: @ref log_error and @ref log_user_error are emitted. */
#define ENGINE_LOG_ERROR 1
/** @brief Includes errors and @ref log_debug messages. */
#define ENGINE_LOG_DEBUG 2
/** @brief Includes every log macro, including @ref log_info. */
#define ENGINE_LOG_INFO 3

#ifndef ENGINE_LOG_LEVEL
#define ENGINE_LOG_LEVEL ENGINE_LOG_INFO
#endif

namespace Engine {

/**
 * @enum LogLevel
 * @brief Severity of one log line.
 *
 * Shared by the terminal writer and the in-memory ring buffer so both stay in sync.
 */
enum class LogLevel { Error, Debug, Info };

/**
 * @enum LogAudience
 * @brief Who a log line is for.
 *
 * Engine lines go to the terminal only. User lines are also stored for the editor Console.
 */
enum class LogAudience { Engine, User };

} // namespace Engine

#endif // ENGINE_LOGGING_LOG_CONFIG_HPP_
