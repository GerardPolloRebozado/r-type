/**
 * @file AError.hpp
 * @brief Abstract implementation of @ref Engine::IError with formatted messages.
 */

#ifndef ENGINE_ERRORS_AERROR_HPP_
#define ENGINE_ERRORS_AERROR_HPP_

#include "Errors/IError.hpp"
#include <string>

namespace Engine {

/**
 * @class AError
 * @brief Base class for concrete Engine errors.
 *
 * Stores a formatted message and severity. Domain-specific error types
 * inherit from this class and pass a type prefix and detail string to the
 * constructor.
 */
class AError : public IError {
  public:
    AError(const std::string& typePrefix, const std::string& detail, error_severity_e severity)
        : m_Message(typePrefix + detail), m_Severity(severity) {}

    const char* what() const noexcept override { return m_Message.c_str(); }

    error_severity_e GetSeverity() const noexcept override { return m_Severity; }

  protected:
    std::string m_Message;
    error_severity_e m_Severity;
};

} // namespace Engine

#endif // ENGINE_ERRORS_AERROR_HPP_
