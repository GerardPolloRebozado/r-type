#include "Core/VirtualResolution.hpp"
#include "Logging/Logger.hpp"
#include <algorithm>
#include <raymath.h>
#include <string>

namespace Engine {

VirtualResolution::VirtualResolution(int width, int height)
    : m_Width(width), m_Height(height), m_Scale(1.0f),
      m_DestRect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)},
      m_MouseMapped(false) {
    Load();
}

VirtualResolution::~VirtualResolution() {
    Unload();
}

int VirtualResolution::ResolveAxis(int virtualSize, int windowSize) {
    if (virtualSize > 0) {
        return virtualSize;
    }
    return windowSize;
}

void VirtualResolution::Load() {
    if (m_Width <= 0 || m_Height <= 0) {
        log_error("VirtualResolution: canvas size must be positive");
        return;
    }

    Unload();
    m_Target = LoadRenderTexture(m_Width, m_Height);
    SetTextureFilter(m_Target.texture, TEXTURE_FILTER_BILINEAR);
}

void VirtualResolution::Unload() {
    if (m_Target.id == 0) {
        return;
    }

    UnloadRenderTexture(m_Target);
    m_Target = RenderTexture2D{};
}

void VirtualResolution::EnsureSize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    if (m_Target.id != 0 && m_Width == width && m_Height == height) {
        return;
    }

    m_Width = width;
    m_Height = height;
    Load();
    log_info("VirtualResolution: canvas " + std::to_string(m_Width) + "x" +
             std::to_string(m_Height));
}

void VirtualResolution::Begin() {
    if (m_Target.id == 0) {
        return;
    }
    BeginTextureMode(m_Target);
}

void VirtualResolution::End() {
    if (m_Target.id == 0) {
        return;
    }
    EndTextureMode();
}

void VirtualResolution::UpdateLetterbox(int screenWidth, int screenHeight) {
    float virtualWidth = static_cast<float>(m_Width);
    float virtualHeight = static_cast<float>(m_Height);
    float windowWidth = static_cast<float>(screenWidth);
    float windowHeight = static_cast<float>(screenHeight);
    float destWidth = 0.0f;
    float destHeight = 0.0f;

    if (virtualWidth <= 0.0f || virtualHeight <= 0.0f) {
        m_Scale = 1.0f;
        m_DestRect = Rectangle{0.0f, 0.0f, windowWidth, windowHeight};
        return;
    }

    // Uniform scale keeps the whole canvas visible; leftover space becomes bars.
    m_Scale = std::min(windowWidth / virtualWidth, windowHeight / virtualHeight);
    destWidth = virtualWidth * m_Scale;
    destHeight = virtualHeight * m_Scale;

    // Split the leftover pixels on both sides so the canvas stays centered.
    m_DestRect = Rectangle{
        (windowWidth - destWidth) * 0.5f,
        (windowHeight - destHeight) * 0.5f,
        destWidth,
        destHeight,
    };
}

void VirtualResolution::Present() {
    Rectangle source = {0.0f, 0.0f, 0.0f, 0.0f};
    Vector2 origin = {0.0f, 0.0f};

    if (m_Target.id == 0) {
        return;
    }

    // Render textures are stored bottom-up; a negative height flips them upright.
    source.width = static_cast<float>(m_Target.texture.width);
    source.height = -static_cast<float>(m_Target.texture.height);
    DrawTexturePro(m_Target.texture, source, m_DestRect, origin, 0.0f, WHITE);
}

Vector2 VirtualResolution::GetVirtualMouse() const {
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = {0.0f, 0.0f};
    Vector2 canvasMin = {0.0f, 0.0f};
    Vector2 canvasMax = {static_cast<float>(m_Width), static_cast<float>(m_Height)};

    if (m_MouseMapped || m_Scale == 0.0f) {
        virtualMouse = mouse;
    } else {
        // Undo the letterbox offset and scale so the cursor lands on the canvas.
        virtualMouse.x = (mouse.x - m_DestRect.x) / m_Scale;
        virtualMouse.y = (mouse.y - m_DestRect.y) / m_Scale;
    }

    return Vector2Clamp(virtualMouse, canvasMin, canvasMax);
}

void VirtualResolution::ApplyMouseMapping() {
    float inverseScale = 1.0f;

    if (m_Scale != 0.0f) {
        inverseScale = 1.0f / m_Scale;
    }

    // Same transform as the letterbox: shift by the bar offset, then scale into canvas pixels.
    SetMouseOffset(static_cast<int>(-m_DestRect.x), static_cast<int>(-m_DestRect.y));
    SetMouseScale(inverseScale, inverseScale);
    m_MouseMapped = true;
}

void VirtualResolution::ClearMouseMapping() {
    SetMouseOffset(0, 0);
    SetMouseScale(1.0f, 1.0f);
    m_MouseMapped = false;
}

} // namespace Engine
