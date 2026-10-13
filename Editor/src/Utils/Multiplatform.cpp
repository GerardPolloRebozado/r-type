#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <stdexcept>

namespace Editor {
std::filesystem::path ConfigDirectory() {
#ifdef _WIN32
    if (const char* appData = std::getenv("APPDATA")) {
        return std::filesystem::path(appData) / "rtype";
    }
    throw std::runtime_error("APPDATA is not set");

#elif defined(__APPLE__)
    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / "Library" / "Application Support" / "rtype";
    }
    throw std::runtime_error("HOME is not set");

#else // Linux and other Unix-like systems
    if (const char* xdgConfig = std::getenv("XDG_CONFIG_HOME"); xdgConfig && *xdgConfig) {
        return std::filesystem::path(xdgConfig) / "rtype";
    }

    if (const char* home = std::getenv("HOME")) {
        return std::filesystem::path(home) / ".config" / "rtype";
    }
    throw std::runtime_error("Neither XDG_CONFIG_HOME nor HOME is set");
#endif
}

std::string FormatDate(const std::string& timestamp) {
    const std::time_t time = static_cast<std::time_t>(std::stoll(timestamp));
    std::tm date{};

#ifdef _WIN32
    localtime_s(&date, &time);
#else
    localtime_r(&time, &date);
#endif

    std::ostringstream output;
    output << std::put_time(&date, "%Y-%m-%d");
    return output.str();
}

} // namespace Editor
