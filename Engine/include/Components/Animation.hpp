#ifndef ENGINE_COMPONENTS_ANIMATION_HPP_
#define ENGINE_COMPONENTS_ANIMATION_HPP_

#include "Assets/Sprites.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Engine {

/**
 * @struct FrameData
 * @brief Represents an individual frame rectangle in a spritesheet and optional pivot offset.
 */
struct FrameData {
    raylib::Rectangle rect; /**< Source rectangle within the spritesheet (x, y, width, height). */
    float pivotX = 0.0f; /**< Pivot / anchor offset X to align sprites with different dimensions. */
    float pivotY = 0.0f; /**< Pivot / anchor offset Y to align sprites with different dimensions. */

    FrameData() : rect(0.0f, 0.0f, 0.0f, 0.0f), pivotX(0.0f), pivotY(0.0f) {}
    FrameData(const raylib::Rectangle& r, float px = 0.0f, float py = 0.0f)
        : rect(r), pivotX(px), pivotY(py) {}
    FrameData(float x, float y, float w, float h, float px = 0.0f, float py = 0.0f)
        : rect(x, y, w, h), pivotX(px), pivotY(py) {}
};

/**
 * @struct Animation
 * @brief Component that holds sprite frames and state for 2D animation.
 *
 * Supports three workflows:
 * 1. Multiple image files (assetPaths has multiple entries).
 * 2. Uniform spritesheet in a single image (frameWidth > 0, frameHeight > 0).
 * 3. Arbitrary spritesheet with custom frames and pivots (frameData list).
 */
struct Animation {
    /** @brief Project-relative paths of the animation frames, or single path if using a
     * spritesheet. */
    std::vector<std::string> assetPaths;

    std::vector<std::string> names;

    /** @brief Cached sprite instances loaded from assetPaths. */
    std::vector<std::shared_ptr<Sprite>> sprites;

    /** @brief Fixed width of each frame when using uniform spritesheet slicing (0 = use full
     * sprite). */
    float frameWidth = 0.0f;

    /** @brief Fixed height of each frame when using uniform spritesheet slicing (0 = use full
     * sprite). */
    float frameHeight = 0.0f;

    /** @brief Total number of frames (useful when slicing a single spritesheet texture). */
    std::size_t totalFrames = 0;

    /** @brief Explicit frame rectangles and pivot offsets for variable-sized spritesheets. */
    std::vector<FrameData> frameData;

    /** @brief Duration of each frame in seconds. */
    float frameDuration = 0.1f;

    /** @brief Elapsed timer for the current frame. */
    float timer = 0.0f;

    /** @brief Index of the currently active frame. */
    std::size_t currentFrame = 0;

    /** @brief Whether the animation loops continuously. */
    bool loop = true;

    /** @brief Whether the animation is currently playing. */
    bool isPlaying = true;

    /** @brief Horizontal offset from Position.x to the sprite's anchor. */
    float offsetX = 0.0f;

    /** @brief Vertical offset from Position.y to the sprite's anchor. */
    float offsetY = 0.0f;

    /** @brief Horizontal scale multiplier of the sprite. */
    float scaleX = 1.0f;

    /** @brief Vertical scale multiplier of the sprite. */
    float scaleY = 1.0f;

    /** @brief Mirrors the sprite horizontally about its center. */
    bool flipX = false;

    /** @brief Mirrors the sprite vertically about its center. */
    bool flipY = false;

    Animation() = default;

    /** @brief Constructor for multi-file frame animation. */
    Animation(std::vector<std::string> paths, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false, std::string name = "Animator")
        : assetPaths(std::move(paths)), frameDuration(frameDuration), loop(loop), offsetX(offsetX),
          offsetY(offsetY), scaleX(scaleX), scaleY(scaleY), flipX(flipX), flipY(flipY) {
        names.push_back(name);
    }

    /** @brief Constructor for fixed-size spritesheet animation. */
    Animation(const std::string& sheetPath, float frameWidth, float frameHeight,
              std::size_t totalFrames = 0, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false, std::string name = "Animator")
        : assetPaths({sheetPath}), frameWidth(frameWidth), frameHeight(frameHeight),
          totalFrames(totalFrames), frameDuration(frameDuration), loop(loop), offsetX(offsetX),
          offsetY(offsetY), scaleX(scaleX), scaleY(scaleY), flipX(flipX), flipY(flipY) {
        names.push_back(name);
    }

    /** @brief Constructor for custom FrameData spritesheet animation. */
    Animation(const std::string& sheetPath, std::vector<FrameData> frames,
              float frameDuration = 0.1f, bool loop = true, float offsetX = 0.0f,
              float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, bool flipX = false,
              bool flipY = false, std::string name = "Animator")
        : assetPaths({sheetPath}), frameData(std::move(frames)), frameDuration(frameDuration),
          loop(loop), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX), scaleY(scaleY),
          flipX(flipX), flipY(flipY) {
        names.push_back(name);
    }
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_ANIMATION_HPP_
