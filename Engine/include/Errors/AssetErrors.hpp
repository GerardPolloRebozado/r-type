/**
 * @file AssetErrors.hpp
 * @brief Asset-related exceptions.
 */

#ifndef ENGINE_ERRORS_ASSET_ERRORS_HPP_
#define ENGINE_ERRORS_ASSET_ERRORS_HPP_

#include "Errors/AError.hpp"
#include <string>

namespace Engine {

/**
 * @class ErrorAsset
 * @brief Exception thrown when an asset cannot be loaded, found, or used.
 */
class ErrorAsset : public AError {
  public:
    explicit ErrorAsset(const std::string& detail,
                        error_severity_e severity = error_severity_e::CRITICAL)
        : AError("Type: Asset\nMessage: ", detail, severity) {}
};

} // namespace Engine

#endif // ENGINE_ERRORS_ASSET_ERRORS_HPP_
