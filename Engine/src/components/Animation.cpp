#include "Components/Animation.hpp"
#include <utility>

namespace Engine {

FrameData::FrameData(const raylib::Rectangle& r, float px, float py)
    : rect(r), pivotX(px), pivotY(py) {}

FrameData::FrameData(float x, float y, float w, float h, float px, float py)
    : rect(x, y, w, h), pivotX(px), pivotY(py) {}

AnimationClip::AnimationClip(std::string clipName, std::vector<std::string> paths, float duration,
                             bool isLooping)
    : name(std::move(clipName)), assetPaths(std::move(paths)), frameDuration(duration),
      loop(isLooping) {}

AnimationClip::AnimationClip(std::string clipName, const std::string& sheetPath, float width,
                             float height, std::size_t total, float duration, bool isLooping)
    : name(std::move(clipName)), assetPaths({sheetPath}), frameWidth(width), frameHeight(height),
      totalFrames(total), frameDuration(duration), loop(isLooping) {}

AnimationClip::AnimationClip(std::string clipName, const std::string& sheetPath,
                             std::vector<FrameData> frames, float duration, bool isLooping)
    : name(std::move(clipName)), assetPaths({sheetPath}), frameData(std::move(frames)),
      frameDuration(duration), loop(isLooping) {}

const AnimationClip* Animation::GetCurrentClip() const {
    auto clip = clips.find(currentAnimation);
    return (clip != clips.end()) ? &clip->second : nullptr;
}

AnimationClip* Animation::GetCurrentClip() {
    auto clip = clips.find(currentAnimation);
    return (clip != clips.end()) ? &clip->second : nullptr;
}

bool Animation::Play(const std::string& clipName, bool restartIfSame) {
    if (clips.find(clipName) == clips.end()) {
        return false;
    }
    if (currentAnimation == clipName && !restartIfSame) {
        return true;
    }
    currentAnimation = clipName;
    currentFrame = 0;
    timer = 0.0f;
    isPlaying = true;
    sprites.clear();
    return true;
}

Animation::Animation(std::vector<std::string> paths, float frameDuration, bool loop, float offsetX,
                     float offsetY, float scaleX, float scaleY, bool flipX, bool flipY,
                     const std::string& clipName)
    : currentAnimation(clipName), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX),
      scaleY(scaleY), flipX(flipX), flipY(flipY) {
    clips[clipName] = AnimationClip(clipName, std::move(paths), frameDuration, loop);
}

Animation::Animation(const std::string& sheetPath, float frameWidth, float frameHeight,
                     std::size_t totalFrames, float frameDuration, bool loop, float offsetX,
                     float offsetY, float scaleX, float scaleY, bool flipX, bool flipY,
                     const std::string& clipName)
    : currentAnimation(clipName), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX),
      scaleY(scaleY), flipX(flipX), flipY(flipY) {
    clips[clipName] = AnimationClip(clipName, sheetPath, frameWidth, frameHeight, totalFrames,
                                    frameDuration, loop);
}

Animation::Animation(const std::string& sheetPath, std::vector<FrameData> frames,
                     float frameDuration, bool loop, float offsetX, float offsetY, float scaleX,
                     float scaleY, bool flipX, bool flipY, const std::string& clipName)
    : currentAnimation(clipName), offsetX(offsetX), offsetY(offsetY), scaleX(scaleX),
      scaleY(scaleY), flipX(flipX), flipY(flipY) {
    clips[clipName] = AnimationClip(clipName, sheetPath, std::move(frames), frameDuration, loop);
}

} // namespace Engine
