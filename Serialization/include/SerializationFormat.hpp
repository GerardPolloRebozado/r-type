#ifndef SERIALIZATION_SERIALIZATION_FORMAT_HPP
#define SERIALIZATION_SERIALIZATION_FORMAT_HPP

#include <string_view>

namespace Serialization {

enum class DataFormat { Auto, Json, Binary };

inline DataFormat DetectFormat(std::string_view buffer) {
    for (char c : buffer) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            continue;
        }
        if (c == '{' || c == '[') {
            return DataFormat::Json;
        }
        break;
    }
    return DataFormat::Binary;
}

} // namespace Serialization

#endif // SERIALIZATION_SERIALIZATION_FORMAT_HPP
