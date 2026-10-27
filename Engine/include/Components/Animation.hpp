#ifndef ENGINE_COMPONENTS_ANIMATION_HPP_
#define ENGINE_COMPONENTS_ANIMATION_HPP_

#include "Assets/Sprites.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Engine {

struct FrameData {
    raylib::Rectangle rect;
    float pivotX = 0.0f;
    float pivotY = 0.0f;

    FrameData() : rect(0.0f, 0.0f, 0.0f, 0.0f), pivotX(0.0f), pivotY(0.0f) {}
    FrameData(const raylib::Rectangle& r, float px = 0.0f, float py = 0.0f)
        : rect(r), pivotX(px), pivotY(py) {}
    FrameData(float x, float y, float w, float h, float px = 0.0f, float py = 0.0f)
        : rect(x, y, w, h), pivotX(px), pivotY(py) {}
};

struct Animation {
    std::vector<std::string> assetPaths;
    std::vector<std::shared_ptr<Sprite>> sprites;
    float frameWidth = 0.0f;
    float frameHeight = 0.0f;
    std::size_t totalFrames = 0;
    std::vector<FrameData> frameData;
    float frameDuration = 0.1f;
    float timer = 0.0f;
    std::size_t currentFrame = 0;
    bool loop = true;
    bool isPlaying = true;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    bool flipX = false;
    bool flipY = false;

    Animation() = default;
    /** @brief constructor for multifile sprite animation*/
    Animation(std::vector<std::string> paths, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false)
        : assetPaths(std::move(paths)), frameDuration(frameDuration), loop(loop), offsetX(offsetX),
          offsetY(offsetY), scaleX(scaleX), scaleY(scaleY), flipX(flipX), flipY(flipY) {}

    /** @brief Constructor for fixed-size spritesheet animation. */
    Animation(const std::string& sheetPath, float frameWidth, float frameHeight,
              std::size_t totalFrames = 0, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false)
        : assetPaths({sheetPath}), frameWidth(frameWidth), frameHeight(frameHeight),
          totalFrames(totalFrames), frameDuration(frameDuration), loop(loop), offsetX(offsetX),
          offsetY(offsetY), scaleX(scaleX), scaleY(scaleY), flipX(flipX), flipY(flipY) {}

    /** @brief Constructor for custom FrameData spritesheet animation. */
    Animation(const std::string& sheetPath, std::vector<FrameData> frames,
              float frameDuration = 0.1f, bool loop = true, float offsetX = 0.0f,
              float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, bool flipX = false,
              bool flipY = false)
        : assetPaths({sheetPath}), frameData(std::move(frames)), frameDuration(frameDuration),
          loop(loop), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX), scaleY(scaleY),
          flipX(flipX), flipY(flipY) {}
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_ANIMATION_HPP_
