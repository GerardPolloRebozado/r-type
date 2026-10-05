#ifndef ENGINE_COMPONENTS_AUDIOSOURCE_HPP_
#define ENGINE_COMPONENTS_AUDIOSOURCE_HPP_

#include <string>

namespace Engine {
/**
 * @struct AudioSource
 * @brief A pure ECS data component for triggering audio playback, when data is modified the audio
 * will read to use using the Audio system
 */
struct AudioSource {
    std::string assetId;
    bool isMusic = false; // True if it's a music stream, False if it's a sound effect

    // Triggers (Systems react to these and reset them)
    bool shouldPlay = false;
    bool shouldStop = false;

    // State (Systems update this to reflect reality)
    bool isPlaying = false;

    bool loop = false;
    float volume = 1.0f;
};
} // namespace Engine

#endif // ENGINE_COMPONENTS_AUDIOSOURCE_HPP_