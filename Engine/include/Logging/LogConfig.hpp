/**
 * @file LogConfig.hpp
 * @brief Compile-time log level constants for the Engine logger.
 *
 * Log verbosity is controlled at build time via the CMake cache variable
 * @c ENGINE_LOG_LEVEL (passed as a compile definition on the @c Engine target).
 */

#ifndef ENGINE_LOGGING_LOG_CONFIG_HPP_
#define ENGINE_LOGGING_LOG_CONFIG_HPP_

/** @brief Minimum level: only @ref log_error messages are emitted. */
#define ENGINE_LOG_ERROR 1
/** @brief Includes @ref log_error and @ref log_debug messages. */
#define ENGINE_LOG_DEBUG 2
/** @brief Includes all log macros (@ref log_error, @ref log_debug, @ref log_info). */
#define ENGINE_LOG_INFO 3

#ifndef ENGINE_LOG_LEVEL
#define ENGINE_LOG_LEVEL ENGINE_LOG_INFO
#endif

#endif // ENGINE_LOGGING_LOG_CONFIG_HPP_
