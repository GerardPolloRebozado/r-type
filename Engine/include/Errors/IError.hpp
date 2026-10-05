/**
 * @file IError.hpp
 * @brief Interface for typed exceptions used across the Engine.
 */

#ifndef ENGINE_ERRORS_IERROR_HPP_
#define ENGINE_ERRORS_IERROR_HPP_

#include <exception>

namespace Engine {

/**
 * @enum error_severity_e
 * @brief Severity level attached to an Engine error.
 */
enum class error_severity_e { LOW, MODERATE, CRITICAL, FATAL };

/**
 * @class IError
 * @brief Abstract base interface for all Engine exceptions.
 *
 * Inherits @c std::exception so existing @c catch (const std::exception&) handlers
 * keep working. Adds a severity hint for callers that need finer handling.
 */
class IError : public std::exception {
  public:
    ~IError() override = default;

    const char* what() const noexcept override = 0;
    virtual error_severity_e GetSeverity() const noexcept = 0;
};

} // namespace Engine

#endif // ENGINE_ERRORS_IERROR_HPP_
