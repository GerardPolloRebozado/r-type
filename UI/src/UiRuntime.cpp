#include "UI/UiRuntime.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Core/VirtualResolution.hpp"
#include "Logging/Logger.hpp"
#include "UI/UiSystem.hpp"
#include <algorithm>
#include <filesystem>
#include <optional>
#include <rlImGui.h>
#include <typeindex>

namespace Engine {
namespace {

ImVec4 Lighten(ImVec4 color) {
    color.x = std::min(color.x + 0.12f, 1.0f);
    color.y = std::min(color.y + 0.12f, 1.0f);
    color.z = std::min(color.z + 0.12f, 1.0f);
    return color;
}

ImVec4 Darken(ImVec4 color) {
    color.x = std::max(color.x - 0.12f, 0.0f);
    color.y = std::max(color.y - 0.12f, 0.0f);
    color.z = std::max(color.z - 0.12f, 0.0f);
    return color;
}

std::optional<ImVec4> ReadColor(const sol::table& props, const char* key) {
    sol::optional<sol::table> color = props[key];
    if (!color || !color->valid()) {
        return std::nullopt;
    }
    ImVec4 value{1.0f, 1.0f, 1.0f, 1.0f};
    value.x = color->get_or(1, 1.0f);
    value.y = color->get_or(2, 1.0f);
    value.z = color->get_or(3, 1.0f);
    value.w = color->get_or(4, 1.0f);
    return value;
}

} // namespace

UiRuntime::~UiRuntime() {
    m_Textures.clear();
    if (m_ImGuiReady && IsWindowReady()) {
        rlImGuiShutdown();
        m_ImGuiReady = false;
    }
}

UiRuntime::GpuTexture::~GpuTexture() {
    if (texture.id != 0 && IsWindowReady()) {
        UnloadTexture(texture);
        texture.id = 0;
    }
}

void UiRuntime::SetTemplateRoot(std::string root) {
    m_TemplateRoot = std::move(root);
}

void UiRuntime::EnsureImGui() {
    if (m_ImGuiReady) {
        return;
    }
    rlImGuiBeginInitImGui();
    rlImGuiEndInitImGui();
    m_ImGuiReady = true;
    ApplyTheme();
    m_ThemeDirty = false;
}

void UiRuntime::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = nullptr;
    ImVec4 border = m_Theme.accent;

    style.WindowRounding = m_Theme.rounding;
    style.FrameRounding = m_Theme.rounding;
    style.GrabRounding = m_Theme.rounding;
    style.WindowPadding = ImVec2(18.0f, 16.0f);
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);

    colors = style.Colors;
    border.w = 0.35f;
    colors[ImGuiCol_Text] = m_Theme.text;
    colors[ImGuiCol_WindowBg] = m_Theme.panel;
    colors[ImGuiCol_PopupBg] = m_Theme.panel;
    colors[ImGuiCol_Border] = border;
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.28f, 0.34f, 1.0f);
    colors[ImGuiCol_TitleBg] = m_Theme.panel;
    colors[ImGuiCol_TitleBgActive] = m_Theme.panel;
    colors[ImGuiCol_Button] = m_Theme.accent;
    colors[ImGuiCol_ButtonHovered] = Lighten(m_Theme.accent);
    colors[ImGuiCol_ButtonActive] = Darken(m_Theme.accent);
    colors[ImGuiCol_Header] = m_Theme.accent;
    colors[ImGuiCol_HeaderHovered] = Lighten(m_Theme.accent);
    colors[ImGuiCol_HeaderActive] = Darken(m_Theme.accent);
    colors[ImGuiCol_SliderGrab] = m_Theme.accent;
    colors[ImGuiCol_SliderGrabActive] = Lighten(m_Theme.accent);
    colors[ImGuiCol_CheckMark] = m_Theme.accent;
    ImGui::GetIO().FontGlobalScale = m_Theme.fontScale;
}

void UiRuntime::ReleaseStyles() {
    while (!m_StyleStack.empty()) {
        PopStyle();
    }
}

void UiRuntime::Draw(Registry& registry) {
    const auto& systems = registry.GetSystems();
    std::unordered_map<std::type_index, std::shared_ptr<System>>::const_iterator system = systems.end();

    if (!registry.HasResource<VirtualResolution>()) {
        return;
    }
    registry.GetResource<VirtualResolution>().ClearMouseMapping();
    EnsureImGui();
    if (!m_ImGuiReady) {
        return;
    }

    rlImGuiBegin();
    if (m_ThemeDirty) {
        ApplyTheme();
        m_ThemeDirty = false;
    }

    system = systems.find(typeid(UiSystem));
    if (system != systems.end()) {
        std::static_pointer_cast<UiSystem>(system->second)->Draw(registry);
    }

    m_BlocksMouse = ImGui::GetIO().WantCaptureMouse;
    m_BlocksKeyboard = ImGui::GetIO().WantCaptureKeyboard;
    ReleaseStyles();
    rlImGuiEnd();
}

void UiRuntime::SetTheme(const sol::table& props) {
    std::optional<ImVec4> accent = ReadColor(props, "accent");
    std::optional<ImVec4> panel = ReadColor(props, "panel");
    std::optional<ImVec4> text = ReadColor(props, "text");
    sol::optional<float> rounding = props["rounding"];
    sol::optional<float> fontScale = props["fontScale"];

    if (accent) {
        m_Theme.accent = *accent;
    }
    if (panel) {
        m_Theme.panel = *panel;
    }
    if (text) {
        m_Theme.text = *text;
    }
    if (rounding) {
        m_Theme.rounding = *rounding;
    }
    if (fontScale) {
        m_Theme.fontScale = *fontScale;
    }
    m_ThemeDirty = true;
    if (m_ImGuiReady) {
        ApplyTheme();
        m_ThemeDirty = false;
    }
}

void UiRuntime::PushStyle(const sol::table& props) {
    StyleFrame frame;
    ImVec4 button = m_Theme.accent;
    ImVec4 hover = Lighten(m_Theme.accent);
    ImVec4 active = Darken(m_Theme.accent);
    ImVec4 text = m_Theme.text;
    std::string styleName = "primary";
    sol::optional<std::string> style = props["style"];
    std::optional<ImVec4> color = ReadColor(props, "color");
    std::optional<ImVec4> hoverColor = ReadColor(props, "hover");
    std::optional<ImVec4> activeColor = ReadColor(props, "active");
    std::optional<ImVec4> textColor = ReadColor(props, "text");
    sol::optional<float> rounding = props["rounding"];
    sol::optional<float> fontScale = props["fontScale"];
    sol::optional<bool> enabled = props["enabled"];

    if (style) {
        styleName = *style;
    }
    if (styleName == "secondary") {
        button = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);
        hover = Lighten(button);
        active = Darken(button);
    } else if (styleName == "ghost") {
        button = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        hover = ImVec4(m_Theme.accent.x, m_Theme.accent.y, m_Theme.accent.z, 0.18f);
        active = ImVec4(m_Theme.accent.x, m_Theme.accent.y, m_Theme.accent.z, 0.30f);
        text = m_Theme.accent;
    } else if (styleName == "danger") {
        button = ImVec4(0.75f, 0.22f, 0.24f, 1.0f);
        hover = ImVec4(0.86f, 0.30f, 0.32f, 1.0f);
        active = ImVec4(0.58f, 0.14f, 0.16f, 1.0f);
    }
    if (color) {
        button = *color;
    }
    if (hoverColor) {
        hover = *hoverColor;
    }
    if (activeColor) {
        active = *activeColor;
    }
    if (textColor) {
        text = *textColor;
    }

    if (m_ImGuiReady) {
        ImGui::PushStyleColor(ImGuiCol_Button, button);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);
        ImGui::PushStyleColor(ImGuiCol_Text, text);
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, button);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, hover);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, button);
        frame.colors = 7;
        if (rounding) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, *rounding);
            frame.vars = 1;
        }
        if (fontScale) {
            frame.previousFontScale = ImGui::GetIO().FontGlobalScale;
            ImGui::GetIO().FontGlobalScale = *fontScale;
            frame.fontChanged = true;
        }
        if (enabled && !*enabled) {
            ImGui::BeginDisabled();
            frame.disabled = true;
        }
    }
    m_StyleStack.push_back(frame);
}

void UiRuntime::PopStyle() {
    StyleFrame frame;

    if (m_StyleStack.empty()) {
        return;
    }
    frame = m_StyleStack.back();
    m_StyleStack.pop_back();
    if (!m_ImGuiReady) {
        return;
    }
    if (frame.disabled) {
        ImGui::EndDisabled();
    }
    if (frame.fontChanged) {
        ImGui::GetIO().FontGlobalScale = frame.previousFontScale;
    }
    if (frame.vars > 0) {
        ImGui::PopStyleVar(frame.vars);
    }
    if (frame.colors > 0) {
        ImGui::PopStyleColor(frame.colors);
    }
}

bool UiRuntime::BlocksMouse() const {
    return m_BlocksMouse;
}

bool UiRuntime::BlocksKeyboard() const {
    return m_BlocksKeyboard;
}

sol::table UiRuntime::UseTemplate(sol::state_view lua, Registry& registry, const std::string& name) {
    std::unordered_map<std::string, sol::table>::iterator cached = m_Templates.find(name);
    std::filesystem::path path;
    sol::protected_function_result result;
    sol::object object;

    if (cached != m_Templates.end()) {
        return cached->second;
    }

    path = ResolveAssetPath(registry, "ui/templates/" + name + ".lua");
    if (!std::filesystem::exists(path)) {
        path = std::filesystem::path(m_TemplateRoot) / (name + ".lua");
    }
    if (!std::filesystem::exists(path)) {
        log_user_error("[UI] Template not found: " + name);
        return lua.create_table();
    }

    result = lua.script_file(path.string(), &sol::script_default_on_error);
    if (!result.valid()) {
        sol::error err = result;
        log_user_error("[UI] Failed to load template " + path.string() + ": " + err.what());
        return lua.create_table();
    }
    object = result;
    if (object.get_type() != sol::type::table) {
        log_user_error("[UI] Template must return a table: " + path.string());
        return lua.create_table();
    }

    sol::table table = object.as<sol::table>();
    m_Templates.emplace(name, table);
    return table;
}

void UiRuntime::RegisterTemplate(const std::string& name, sol::table table) {
    m_Templates[name] = std::move(table);
}

void UiRuntime::DrawImage(Registry& registry, const std::string& path, float width, float height) {
    std::unordered_map<std::string, std::unique_ptr<GpuTexture>>::iterator found = m_Textures.find(path);
    std::filesystem::path full;
    Texture2D loaded{};
    int drawWidth = 0;
    int drawHeight = 0;

    if (path.empty()) {
        return;
    }
    if (found == m_Textures.end()) {
        full = ResolveAssetPath(registry, path);
        if (!std::filesystem::exists(full)) {
            if (m_WarnedImages.insert(path).second) {
                log_user_error("[UI] Texture not found: " + full.string());
            }
            return;
        }
        loaded = LoadTexture(full.string().c_str());
        if (loaded.id == 0) {
            log_user_error("[UI] Failed to load texture: " + full.string());
            return;
        }
        found = m_Textures.emplace(path, std::make_unique<GpuTexture>(loaded)).first;
    }
    if (found->second->texture.id == 0) {
        return;
    }
    drawWidth = static_cast<int>(width);
    drawHeight = static_cast<int>(height);
    if (drawWidth <= 0) {
        drawWidth = found->second->texture.width;
    }
    if (drawHeight <= 0) {
        drawHeight = found->second->texture.height;
    }
    rlImGuiImageSize(&found->second->texture, drawWidth, drawHeight);
}

} // namespace Engine
