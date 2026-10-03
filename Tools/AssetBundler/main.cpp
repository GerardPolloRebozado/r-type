#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

std::string getValidVariableName(std::string name) {
    for (char &c: name) {
        if (!isalnum(c)) c = '_';
    }
    return name;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <assets_dir> <output.hpp>\n";
        return 1;
    }

    std::string assetsDir = argv[1];
    std::string outputFile = argv[2];

    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Cannot open output file: " << outputFile << "\n";
        return 1;
    }

    out << "#pragma once\n\n";
    out << "#include \"Assets/AssetManager.hpp\"\n\n";
    out << "namespace BundledAssets {\n\n";

    struct AssetInfo {
        std::string varName;
        std::string extension;
        std::string alias;
    };
    std::vector<AssetInfo> assets;

    if (fs::exists(assetsDir) && fs::is_directory(assetsDir)) {
        for (const auto &entry: fs::recursive_directory_iterator(assetsDir)) {
            if (!entry.is_regular_file()) continue;

            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();
            std::string filename = entry.path().filename().string();

            std::string alias = entry.path().lexically_relative(assetsDir).string();
            std::replace(alias.begin(), alias.end(), '\\', '/');

            std::string varName = getValidVariableName(alias);

            std::ifstream in(path, std::ios::binary);
            if (!in) {
                std::cerr << "Warning: Could not read file " << path << "\n";
                continue;
            }

            out << "inline const unsigned char " << varName << "[] = {\n    ";

            char c;
            int count = 0;
            while (in.get(c)) {
                out << "0x" << std::hex << (static_cast<int>(c) & 0xff) << ", ";
                if (++count % 12 == 0) out << "\n    ";
            }
            out << std::dec << "\n};\n";
            out << "inline const int " << varName << "_size = " << count << ";\n\n";

            assets.push_back({varName, ext, alias});
        }
    } else {
        std::cerr << "Warning: Assets directory " << assetsDir << " does not exist or is not a directory.\n";
    }

    out << "inline void RegisterAssets(Engine::AssetManager& manager) {\n";
    for (const auto &a: assets) {
        if (a.extension == ".wav" || a.extension == ".mp3" || a.extension == ".ogg" || a.extension == ".flac") {
            if (a.alias.find("music") != std::string::npos || a.alias.find("bgm") != std::string::npos) {
                out << "    manager.LoadMusicFromMemory(\"" << a.alias << "\", \"" << a.extension << "\", "
                        << a.varName << ", " << a.varName << "_size);\n";
            } else {
                out << "    manager.LoadSoundFromMemory(\"" << a.alias << "\", \"" << a.extension << "\", "
                        << a.varName << ", " << a.varName << "_size);\n";
            }
        }
        // Additional asset types (Fonts, Textures) can be added here when AssetManager supports them
        if (a.extension == ".ttf" || a.extension == ".otf") {
            out << "    manager.LoadFontFromMemory(\"" << a.alias << "\", \"" << a.extension << "\", "
                    << a.varName << ", " << a.varName << "_size);\n";
        }
    }
    out << "}\n\n";
    out << "} // namespace BundledAssets\n";

    std::cout << "Assets bundled to " << outputFile << " (" << assets.size() << " files)\n";
    return 0;
}
