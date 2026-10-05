/**
 * @file EcsErrors.hpp
 * @brief ECS registry and component exceptions.
 */

#ifndef ENGINE_ERRORS_ECS_ERRORS_HPP_
#define ENGINE_ERRORS_ECS_ERRORS_HPP_

#include "Errors/AError.hpp"
#include <string>

namespace Engine {

/**
 * @class ErrorEcs
 * @brief Exception thrown when ECS invariants are violated (missing component, resource, etc.).
 */
class ErrorEcs : public AError {
  public:
    explicit ErrorEcs(const std::string& detail,
                      error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: ECS\nMessage: ", detail, severity) {}
};

} // namespace Engine

#endif // ENGINE_ERRORS_ECS_ERRORS_HPP_
