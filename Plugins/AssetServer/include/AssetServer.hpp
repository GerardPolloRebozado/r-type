#ifndef PLUGINS_ASSSET_SERVER_PLUGIN_HPP_
#define PLUGINS_ASSSET_SERVER_PLUGIN_HPP_

#include "raylib-cpp.hpp"
#include "Plugins/IPlugin.hpp"
#include <string>
#include <utility>

namespace Engine {
    /**
     * @class AssetServer
     * @brief Asset storage
     */
    class AssetServer : public IPlugin {
    public:
        class FileNotFoundException : std::exception {
            std::string _file;

        public:
            const char *what() const noexcept override {
                return (std::string("The file: ") + _file + std::string("was not found")).c_str();
            }

            FileNotFoundException(std::string file) : _file(std::move(file)) {
            };
        };

        AssetServer() = default;

        ~AssetServer() override = default;

        /**
         * @brief Initializes the sound plugin with the ECS registry
ry.
         * @param registry A reference to the main ECS registry.
         */
        void Init(Registry &registry) override;
    };
} // namespace Engine

#endif // PLUGINS_SOUND_SOUNDPLUGIN_HPP_
