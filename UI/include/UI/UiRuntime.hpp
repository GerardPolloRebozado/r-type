#ifndef UI_UIRUNTIME_HPP_
#define UI_UIRUNTIME_HPP_

#include "ECS/Registry.hpp"
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

    std::string m_TemplateRoot;
    bool m_ImGuiReady = false;
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
