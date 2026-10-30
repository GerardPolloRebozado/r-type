#ifndef ENGINE_COMPONENTS_ANIMATION_HPP_
#define ENGINE_COMPONENTS_ANIMATION_HPP_

#include "Assets/Sprites.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

/**
 * @struct FrameData
 * @brief Represents an individual frame rectangle in a spritesheet and optional pivot offset.
 */
struct FrameData {
    raylib::Rectangle rect; /**< Sub-rectangle within the texture atlas (x, y, width, height). */
    float pivotX = 0.0f;    /**< Horizontal alignment offset relative to the entity's position. */
    float pivotY = 0.0f;    /**< Vertical alignment offset relative to the entity's position. */

    /** @brief Default constructor initializing an empty frame. */
    FrameData() = default;

    /**
     * @brief Constructs frame data using a Raylib rectangle and optional pivots.
     * @param r Sub-rectangle within the texture.
     * @param px Horizontal pivot offset.
     * @param py Vertical pivot offset.
     */
    FrameData(const raylib::Rectangle& r, float px = 0.0f, float py = 0.0f);

    /**
     * @brief Constructs frame data using raw coordinates and dimensions.
     * @param x X coordinate of the top-left corner in the texture.
     * @param y Y coordinate of the top-left corner in the texture.
     * @param w Width of the frame rectangle.
     * @param h Height of the frame rectangle.
     * @param px Horizontal pivot offset.
     * @param py Vertical pivot offset.
     */
    FrameData(float x, float y, float w, float h, float px = 0.0f, float py = 0.0f);
};

/**
 * @struct AnimationClip
 * @brief Encapsulates static configuration data for a single animation state.
 *
 * Stores frame sources, slicing geometry, timing, and looping settings.
 * Supports three workflows:
 * 1. Multiple discrete file paths (`assetPaths`).
 * 2. Uniform spritesheet slicing (`frameWidth` x `frameHeight`).
 * 3. Custom variable-sized frames (`frameData`).
 */
struct AnimationClip {
    std::string name = "Idle";           /**< Unique identifier name of the clip. */
    std::vector<std::string> assetPaths; /**< Paths to image assets or a single spritesheet. */
    float frameWidth = 0.0f;     /**< Width of each frame for uniform slicing (0 = full sprite). */
    float frameHeight = 0.0f;    /**< Height of each frame for uniform slicing (0 = full sprite). */
    std::size_t totalFrames = 0; /**< Total frame count override (0 = auto-calculate). */
    std::vector<FrameData> frameData; /**< Explicit frame rectangles for variable spritesheets. */
    float frameDuration = 0.1f;       /**< Playback time per frame in seconds. */
    bool loop = true;                 /**< Indicates whether playback repeats after finishing. */

    /** @brief Default constructor. */
    AnimationClip() = default;

    /**
     * @brief Constructs a clip from multiple individual image files.
     * @param clipName Identifier for the animation clip.
     * @param paths List of asset paths representing sequential frames.
     * @param duration Display time per frame in seconds.
     * @param isLooping Whether the animation loops continuously.
     */
    AnimationClip(std::string clipName, std::vector<std::string> paths, float duration = 0.1f,
                  bool isLooping = true);

    /**
     * @brief Constructs a clip for uniform grid-based spritesheets.
     * @param clipName Identifier for the animation clip.
     * @param sheetPath Path to the spritesheet texture asset.
     * @param width Width of a single frame cell in pixels.
     * @param height Height of a single frame cell in pixels.
     * @param total Total number of frames in the grid (0 = auto-calculate).
     * @param duration Display time per frame in seconds.
     * @param isLooping Whether the animation loops continuously.
     */
    AnimationClip(std::string clipName, const std::string& sheetPath, float width, float height,
                  std::size_t total = 0, float duration = 0.1f, bool isLooping = true);

    /**
     * @brief Constructs a clip using explicit custom FrameData definitions.
     * @param clipName Identifier for the animation clip.
     * @param sheetPath Path to the texture atlas asset.
     * @param frames Array of explicit frame rectangles and pivots.
     * @param duration Display time per frame in seconds.
     * @param isLooping Whether the animation loops continuously.
     */
    AnimationClip(std::string clipName, const std::string& sheetPath, std::vector<FrameData> frames,
                  float duration = 0.1f, bool isLooping = true);
};

/**
 * @struct Animation
 * @brief ECS Component managing animation clip collections and runtime playback state.
 *
 * Contains all animation clips assigned to an entity, runtime playback counters,
 * texture instance caches, and spatial transformations (scale, offset, mirroring).
 */
struct Animation {
    std::unordered_map<std::string, AnimationClip>
        clips;                         /**< Map of clip names to their static configuration. */
    std::string currentAnimation = ""; /**< Key of the currently active animation clip. */
    std::vector<std::shared_ptr<Sprite>>
        sprites; /**< Cached runtime sprite instances for active asset paths. */

    float timer = 0.0f;           /**< Accumulated delta time for current frame step. */
    std::size_t currentFrame = 0; /**< Index of the currently displayed frame. */
    bool isPlaying = true;        /**< Controls active playback advancement. */

    float offsetX = 0.0f; /**< Horizontal anchor offset added to Entity Position.x. */
    float offsetY = 0.0f; /**< Vertical anchor offset added to Entity Position.y. */
    float scaleX = 1.0f;  /**< Horizontal scale multiplier. */
    float scaleY = 1.0f;  /**< Vertical scale multiplier. */
    bool flipX = false;   /**< Mirrors sprite horizontally when rendered. */
    bool flipY = false;   /**< Mirrors sprite vertically when rendered. */

    /** @brief Default constructor. */
    Animation() = default;

    /**
     * @brief Gets a const pointer to the currently active AnimationClip.
     * @return Pointer to active AnimationClip, or nullptr if no valid clip is active.
     */
    const AnimationClip* GetCurrentClip() const;

    /**
     * @brief Gets a mutable pointer to the currently active AnimationClip.
     * @return Pointer to active AnimationClip, or nullptr if no valid clip is active.
     */
    AnimationClip* GetCurrentClip();

    /**
     * @brief Switches active animation state and resets playback counters.
     * @param clipName Name key of the clip to play.
     * @param restartIfSame If true, restarts playback from frame 0 even if already playing.
     * @return True if the requested clip exists and was activated, false otherwise.
     */
    bool Play(const std::string& clipName, bool restartIfSame = false);

    /**
     * @brief Helper constructor initializing an Animation component with a multi-file clip.
     * @param paths List of image file paths.
     * @param frameDuration Time per frame in seconds.
     * @param loop Enables continuous looping.
     * @param offsetX World position horizontal offset.
     * @param offsetY World position vertical offset.
     * @param scaleX Horizontal scale factor.
     * @param scaleY Vertical scale factor.
     * @param flipX Horizontal flip flag.
     * @param flipY Vertical flip flag.
     * @param clipName Default clip identifier name.
     */
    Animation(std::vector<std::string> paths, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false, const std::string& clipName = "Idle");

    /**
     * @brief Helper constructor initializing an Animation component with a uniform grid clip.
     * @param sheetPath Path to spritesheet image.
     * @param frameWidth Cell frame width in pixels.
     * @param frameHeight Cell frame height in pixels.
     * @param totalFrames Total frame count.
     * @param frameDuration Time per frame in seconds.
     * @param loop Enables continuous looping.
     * @param offsetX World position horizontal offset.
     * @param offsetY World position vertical offset.
     * @param scaleX Horizontal scale factor.
     * @param scaleY Vertical scale factor.
     * @param flipX Horizontal flip flag.
     * @param flipY Vertical flip flag.
     * @param clipName Default clip identifier name.
     */
    Animation(const std::string& sheetPath, float frameWidth, float frameHeight,
              std::size_t totalFrames = 0, float frameDuration = 0.1f, bool loop = true,
              float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f,
              bool flipX = false, bool flipY = false, const std::string& clipName = "Idle");

    /**
     * @brief Helper constructor initializing an Animation component with explicit FrameData.
     * @param sheetPath Path to texture atlas.
     * @param frames Array of FrameData frame specifications.
     * @param frameDuration Time per frame in seconds.
     * @param loop Enables continuous looping.
     * @param offsetX World position horizontal offset.
     * @param offsetY World position vertical offset.
     * @param scaleX Horizontal scale factor.
     * @param scaleY Vertical scale factor.
     * @param flipX Horizontal flip flag.
     * @param flipY Vertical flip flag.
     * @param clipName Default clip identifier name.
     */
    Animation(const std::string& sheetPath, std::vector<FrameData> frames,
              float frameDuration = 0.1f, bool loop = true, float offsetX = 0.0f,
              float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, bool flipX = false,
              bool flipY = false, const std::string& clipName = "Idle");
};

} // namespace Engine

#endif // ENGINE_COMPONENTS_ANIMATION_HPP_