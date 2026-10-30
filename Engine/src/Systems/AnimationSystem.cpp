#include "Systems/AnimationSystem.hpp"
#include "Assets/AssetManager.hpp"
#include "Components/Animation.hpp"
#include "Components/Opacity.hpp"
#include "Components/Position.hpp"
#include "Components/Rotation.hpp"
#include "Components/ScreenSpace.hpp"
#include "ECS/Registry.hpp"
#include "Plugins/OpacityPlugin.hpp"
#include "Resources/ProjectContext.hpp"
#include <cmath>

namespace Engine {

static std::size_t GetAnimationTotalFrames(const AnimationClip& clip, const Sprite* sprite) {
    if (!clip.frameData.empty()) {
        return clip.frameData.size();
    }
    if (clip.frameWidth > 0.0f && clip.frameHeight > 0.0f) {
        if (clip.totalFrames > 0) {
            return clip.totalFrames;
        }
        if (sprite) {
            int cols = static_cast<int>(sprite->GetTexture().width / clip.frameWidth);
            int rows = static_cast<int>(sprite->GetTexture().height / clip.frameHeight);
            int total = cols * rows;
            return total > 0 ? static_cast<std::size_t>(total) : 1;
        }
        return 1;
    }
    return clip.assetPaths.size();
}

void AnimationSystem::Update(Registry& registry, float dt) {
    if (!registry.IsComponentRegistered<Animation>()) {
        return;
    }

    auto animArray = registry.GetComponentArray<Animation>();
    for (auto& [entity, anim] : *animArray) {
        auto* clip = anim.GetCurrentClip();
        if (!anim.isPlaying || !clip || clip->assetPaths.empty() || clip->frameDuration <= 0.0f) {
            continue;
        }

        const Sprite* currentSpritePtr = nullptr;
        if (!anim.sprites.empty()) {
            std::size_t spriteIdx =
                (clip->frameWidth > 0.0f || !clip->frameData.empty()) ? 0 : anim.currentFrame;
            if (spriteIdx < anim.sprites.size() && anim.sprites[spriteIdx]) {
                currentSpritePtr = anim.sprites[spriteIdx].get();
            }
        }

        std::size_t total = GetAnimationTotalFrames(*clip, currentSpritePtr);
        if (total <= 1) {
            continue;
        }

        anim.timer += dt;
        while (anim.timer >= clip->frameDuration) {
            anim.timer -= clip->frameDuration;
            if (anim.currentFrame + 1 < total) {
                anim.currentFrame++;
            } else if (clip->loop) {
                anim.currentFrame = 0;
            } else {
                anim.isPlaying = false;
                break;
            }
        }
    }
}

bool AnimationSystem::CanDraw(Registry& registry, Entity entity, bool screenSpace) {
    return registry.HasComponent<Animation>(entity) &&
           registry.HasComponent<ScreenSpace>(entity) == screenSpace;
}

void AnimationSystem::DrawEntity(Registry& registry, Entity entity) {
    if (!registry.HasComponent<Animation>(entity) || !registry.HasComponent<Position>(entity)) {
        return;
    }

    auto& position = registry.GetComponent<Position>(entity);
    auto& anim = registry.GetComponent<Animation>(entity);

    auto* clip = anim.GetCurrentClip();
    if (!clip || clip->assetPaths.empty()) {
        return;
    }

    if (anim.sprites.size() != clip->assetPaths.size()) {
        anim.sprites.clear();
        anim.sprites.resize(clip->assetPaths.size(), nullptr);
    }

    bool isSingleSheet =
        (clip->frameWidth > 0.0f && clip->frameHeight > 0.0f) || !clip->frameData.empty();
    std::size_t assetIdx = isSingleSheet ? 0 : anim.currentFrame;

    if (assetIdx >= clip->assetPaths.size()) {
        assetIdx = 0;
    }

    // Lazy load sprite via AssetManager
    if (!anim.sprites[assetIdx] && !clip->assetPaths[assetIdx].empty()) {
        if (registry.HasResource<AssetManager>()) {
            auto& am = registry.GetResource<AssetManager>();
            const std::string& path = clip->assetPaths[assetIdx];
            const std::string resolved = ResolveProjectPath(registry, path).string();
            try {
                am.LoadSprite(path, resolved);
            } catch (...) {
            }
            try {
                anim.sprites[assetIdx] = am.GetSprite(path);
            } catch (...) {
            }
        }
    }

    auto& currentSprite = anim.sprites[assetIdx];
    if (currentSprite) {
        if (anim.scaleX > 0.0f && anim.scaleY > 0.0f) {
            currentSprite->SetScale({anim.scaleX, anim.scaleY});
        }
        if (registry.HasComponent<Rotation>(entity)) {
            currentSprite->SetRotation(registry.GetComponent<Rotation>(entity).angle);
        }
        raylib::Color tint = raylib::Color::White();
        tint.a =
            static_cast<unsigned char>(255.0f * OpacityPlugin::GetEffectiveAlpha(registry, entity));

        // Calculate source rectangle
        const raylib::Rectangle original = currentSprite->GetSourceRect();
        raylib::Rectangle srcRect = original;
        float pivotOffsetX = 0.0f;
        float pivotOffsetY = 0.0f;

        if (!clip->frameData.empty() && anim.currentFrame < clip->frameData.size()) {
            const auto& fd = clip->frameData[anim.currentFrame];
            srcRect = fd.rect;
            pivotOffsetX = fd.pivotX;
            pivotOffsetY = fd.pivotY;
        } else if (clip->frameWidth > 0.0f && clip->frameHeight > 0.0f) {
            int textureWidth = currentSprite->GetTexture().width;
            int cols = std::max(1, static_cast<int>(textureWidth / clip->frameWidth));
            int col = static_cast<int>(anim.currentFrame) % cols;
            int row = static_cast<int>(anim.currentFrame) / cols;
            srcRect = raylib::Rectangle{static_cast<float>(col * clip->frameWidth),
                                        static_cast<float>(row * clip->frameHeight),
                                        clip->frameWidth, clip->frameHeight};
        }

        const raylib::Vector2 drawPosition = {position.x + anim.offsetX + pivotOffsetX,
                                              position.y + anim.offsetY + pivotOffsetY};

        if (anim.flipX) {
            srcRect.width = -std::fabs(srcRect.width);
        }
        if (anim.flipY) {
            srcRect.height = -std::fabs(srcRect.height);
        }

        currentSprite->SetSourceRect(srcRect);
        currentSprite->Draw(drawPosition, tint);
        currentSprite->SetSourceRect(original);
    }
}

} // namespace Engine