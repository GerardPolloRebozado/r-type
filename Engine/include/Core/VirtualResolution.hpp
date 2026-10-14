#ifndef ENGINE_CORE_VIRTUALRESOLUTION_HPP_
#define ENGINE_CORE_VIRTUALRESOLUTION_HPP_

#include <raylib.h>

namespace Engine {

/**
 * @class VirtualResolution
 * @brief Fixed-size game canvas, scaled and centered into the OS window.
 *
 * Draw calls go into an off-screen render texture. The texture is then scaled
 * uniformly so the virtual aspect ratio is preserved (letterbox / pillarbox).
 * Mouse input can be mapped back into that canvas.
 */
class VirtualResolution {
  public:
    /**
     * @brief Creates the canvas and loads its render texture.
     * @param width Virtual canvas width in pixels.
     * @param height Virtual canvas height in pixels.
     */
    VirtualResolution(int width, int height);

    ~VirtualResolution();

    VirtualResolution(const VirtualResolution&) = delete;
    VirtualResolution& operator=(const VirtualResolution&) = delete;
    VirtualResolution(VirtualResolution&&) = delete;
    VirtualResolution& operator=(VirtualResolution&&) = delete;

    /**
     * @brief Uses @p virtualSize when it is positive, otherwise @p windowSize.
     *
     * A non-positive virtual axis means "match the window" so older projects
     * stay 1:1 until they set a distinct canvas size.
     */
    static int ResolveAxis(int virtualSize, int windowSize);

    /**
     * @brief (Re)loads the render texture for the current virtual size.
     *
     * Safe to call again: an existing texture is released first.
     */
    void Load();

    /**
     * @brief Releases the GPU render texture. Safe to call more than once.
     *
     * Must run while the Raylib window (OpenGL context) is still open.
     */
    void Unload();

    /**
     * @brief Recreates the render texture only when the virtual size changes.
     * @param width New virtual width in pixels.
     * @param height New virtual height in pixels.
     */
    void EnsureSize(int width, int height);

    /**
     * @brief Starts drawing into the virtual canvas.
     */
    void Begin();

    /**
     * @brief Stops drawing into the virtual canvas.
     */
    void End();

    /**
     * @brief Recomputes the uniform scale and the centered destination rectangle.
     * @param screenWidth Current OS window width in pixels.
     * @param screenHeight Current OS window height in pixels.
     */
    void UpdateLetterbox(int screenWidth, int screenHeight);

    /**
     * @brief Draws the canvas into the current frame, letterboxed.
     *
     * The caller clears the window first. Does not change the mouse mapping.
     */
    void Present();

    RenderTexture2D& GetTexture() {
        return m_Target;
    }

    const RenderTexture2D& GetTexture() const {
        return m_Target;
    }

    int GetWidth() const {
        return m_Width;
    }

    int GetHeight() const {
        return m_Height;
    }

    float GetScale() const {
        return m_Scale;
    }

    Rectangle GetDestRect() const {
        return m_DestRect;
    }

    /**
     * @brief Mouse position in virtual canvas pixels, clamped to the canvas.
     *
     * When ApplyMouseMapping() is active, Raylib's cursor is already in canvas
     * space. Otherwise the window cursor is remapped from the letterbox.
     */
    Vector2 GetVirtualMouse() const;

    /**
     * @brief Makes Raylib mouse queries return virtual canvas coordinates.
     *
     * Do not use this while immediate-mode UI (ImGui / rlImGui) reads the mouse.
     */
    void ApplyMouseMapping();

    /**
     * @brief Restores Raylib mouse queries to window coordinates.
     */
    void ClearMouseMapping();

  private:
    RenderTexture2D m_Target{};
    int m_Width = 0;
    int m_Height = 0;
    float m_Scale = 1.0f;
    Rectangle m_DestRect{};
    bool m_MouseMapped = false;
};

} // namespace Engine

#endif // ENGINE_CORE_VIRTUALRESOLUTION_HPP_
