#ifndef UI_UIRUNTIME_HPP_
#define UI_UIRUNTIME_HPP_

#include "ECS/Registry.hpp"
#include <array>
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <raylib.h>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Engine {

/**
 * @brief ImGui frame, theme, template cache, and texture cache for one game.
 */
class UiRuntime {
  public:
    UiRuntime() = default;
    ~UiRuntime();

    UiRuntime(const UiRuntime&) = delete;
    UiRuntime& operator=(const UiRuntime&) = delete;

    void SetTemplateRoot(std::string root);

    /** @brief rlImGui frame, then every UiComponent. */
    void Draw(Registry& registry);

    void SetTheme(const sol::table& props);
    void PushStyle(const sol::table& props);
    void PopStyle();

    /** @brief Number of PushStyle frames still open. */
    std::size_t StyleDepth() const;
    /** @brief Pops PushStyle frames until only depth are left. */
    void RestoreStyleDepth(std::size_t depth);

    /** @brief Preview draws screens inside a rectangle of the editor instead of the window. */
    void SetPreview(bool preview, float x, float y, float width, float height);
    bool IsPreview() const;
    /** @brief Game: (0,0,display). Preview: the rectangle above. Order: x, y, width, height. */
    std::array<float, 4> GetScreenRect() const;

    /** @brief Full-screen page. Game: window at the display. Preview: child at the rect. */
    void BeginScreen(const std::string& id, int flags);
    void EndScreen();
    /** @brief Centered panel of that size inside the screen rect (Confirm). */
    void BeginPanel(const std::string& id, float width, float height, int flags);
    void EndPanel();

    bool BlocksMouse() const;
    bool BlocksKeyboard() const;

    sol::table UseTemplate(sol::state_view lua, Registry& registry, const std::string& name);
    void RegisterTemplate(const std::string& name, sol::table table);
    void DrawImage(Registry& registry, const std::string& path, float width, float height);

  private:
    struct StyleFrame {
        int colors = 0;
        int vars = 0;
        bool disabled = false;
        float previousFontScale = 1.0f;
        bool fontChanged = false;
    };

    struct GpuTexture {
        Texture2D texture{};
        ~GpuTexture();
        GpuTexture() = default;
        explicit GpuTexture(Texture2D loaded) : texture(loaded) {}
        GpuTexture(const GpuTexture&) = delete;
        GpuTexture& operator=(const GpuTexture&) = delete;
    };

    struct Theme {
        ImVec4 accent{0.25f, 0.78f, 0.95f, 1.0f};
        ImVec4 panel{0.06f, 0.07f, 0.10f, 0.94f};
        ImVec4 text{0.93f, 0.94f, 0.96f, 1.0f};
        float rounding = 8.0f;
        float fontScale = 1.0f;
    };

    void EnsureImGui();
    void ApplyTheme();
    void ReleaseStyles();
    void BeginRect(const std::string& id, float x, float y, float width, float height, int flags);
    void EndRect();

    std::string m_TemplateRoot;
    bool m_ImGuiReady = false;
    bool m_Preview = false;
    std::array<float, 4> m_PreviewRect{0.0f, 0.0f, 0.0f, 0.0f};
    bool m_ThemeDirty = true;
    bool m_BlocksMouse = false;
    bool m_BlocksKeyboard = false;
    Theme m_Theme;
    std::vector<StyleFrame> m_StyleStack;
    std::unordered_map<std::string, sol::table> m_Templates;
    std::unordered_map<std::string, std::unique_ptr<GpuTexture>> m_Textures;
    std::unordered_set<std::string> m_WarnedImages;
};

} // namespace Engine

#endif // UI_UIRUNTIME_HPP_
