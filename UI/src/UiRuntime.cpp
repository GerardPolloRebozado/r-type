#include "UI/UiRuntime.hpp"
#include "Assets/ProjectPaths.hpp"
#include "Core/VirtualResolution.hpp"
#include "Logging/Logger.hpp"
#include "UI/UiSystem.hpp"
#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
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

enum class NamedStyle { Primary, Secondary, Ghost, Danger };

NamedStyle NamedStyleFrom(const std::string& name) {
    if (name == "secondary") {
        return NamedStyle::Secondary;
    }
    if (name == "ghost") {
        return NamedStyle::Ghost;
    }
    if (name == "danger") {
        return NamedStyle::Danger;
    }
    return NamedStyle::Primary;
}

std::optional<ImVec4> ReadColor(const sol::table& props, const std::string& key) {
    sol::optional<sol::table> color = props[key];
    if (!color || !color->valid()) {
        return std::nullopt;
    }
    ImVec4 value{1.0f, 1.0f, 1.0f, 1.0f};
    // Lua colors are {r, g, b, a} at indexes 1..4.
    value.x = color->get_or(1, 1.0f);
    value.y = color->get_or(2, 1.0f);
    value.z = color->get_or(3, 1.0f);
    value.w = color->get_or(4, 1.0f);
    return value;
}

} // namespace

UiRuntime::~UiRuntime() {
    m_Textures.clear();
    // The window is already closed when the registry is destroyed.
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
    ImVec4 border{0.73f, 0.66f, 0.48f, 0.90f};
    ImVec4 frame = m_Theme.panel;

    // Bone border and a frame slightly lighter than the panel, so widgets stay on the black field.
    frame.x = std::min(frame.x + 0.07f, 1.0f);
    frame.y = std::min(frame.y + 0.06f, 1.0f);
    frame.z = std::min(frame.z + 0.045f, 1.0f);
    frame.w = 1.0f;

    style.WindowRounding = m_Theme.rounding;
    style.FrameRounding = m_Theme.rounding;
    style.GrabRounding = m_Theme.rounding;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(18.0f, 16.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(10.0f, 8.0f);

    style.Colors[ImGuiCol_Text] = m_Theme.text;
    style.Colors[ImGuiCol_WindowBg] = m_Theme.panel;
    style.Colors[ImGuiCol_PopupBg] = m_Theme.panel;
    style.Colors[ImGuiCol_Border] = border;
    style.Colors[ImGuiCol_Separator] = border;
    style.Colors[ImGuiCol_FrameBg] = frame;
    style.Colors[ImGuiCol_FrameBgHovered] = Lighten(frame);
    style.Colors[ImGuiCol_FrameBgActive] = Lighten(Lighten(frame));
    style.Colors[ImGuiCol_TitleBg] = m_Theme.panel;
    style.Colors[ImGuiCol_TitleBgActive] = m_Theme.panel;
    style.Colors[ImGuiCol_Button] = m_Theme.accent;
    style.Colors[ImGuiCol_ButtonHovered] = Lighten(m_Theme.accent);
    style.Colors[ImGuiCol_ButtonActive] = Darken(m_Theme.accent);
    style.Colors[ImGuiCol_Header] = m_Theme.accent;
    style.Colors[ImGuiCol_HeaderHovered] = Lighten(m_Theme.accent);
    style.Colors[ImGuiCol_HeaderActive] = Darken(m_Theme.accent);
    style.Colors[ImGuiCol_SliderGrab] = m_Theme.accent;
    style.Colors[ImGuiCol_SliderGrabActive] = Lighten(m_Theme.accent);
    style.Colors[ImGuiCol_CheckMark] = m_Theme.accent;
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
    // The letterbox maps the mouse into virtual pixels. ImGui reads the real window.
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

    // GetSystem throws if the system was never registered.
    system = systems.find(typeid(UiSystem));
    if (system != systems.end()) {
        std::static_pointer_cast<UiSystem>(system->second)->Draw(registry);
    }

    m_BlocksMouse = ImGui::GetIO().WantCaptureMouse;
    m_BlocksKeyboard = ImGui::GetIO().WantCaptureKeyboard;
    // A Lua error can skip popStyle. Drop anything still pushed.
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
    // Primary is already copied from the theme. An unknown name stays there.
    switch (NamedStyleFrom(styleName)) {
    case NamedStyle::Secondary:
        button = ImVec4(0.18f, 0.22f, 0.28f, 1.0f);
        hover = Lighten(button);
        active = Darken(button);
        break;
    case NamedStyle::Ghost:
        button = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        hover = ImVec4(m_Theme.accent.x, m_Theme.accent.y, m_Theme.accent.z, 0.18f);
        active = ImVec4(m_Theme.accent.x, m_Theme.accent.y, m_Theme.accent.z, 0.30f);
        text = m_Theme.accent;
        break;
    case NamedStyle::Danger:
        button = ImVec4(0.75f, 0.22f, 0.24f, 1.0f);
        hover = ImVec4(0.86f, 0.30f, 0.32f, 1.0f);
        active = ImVec4(0.58f, 0.14f, 0.16f, 1.0f);
        break;
    case NamedStyle::Primary:
        break;
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

    // The editor already has an ImGui context. m_ImGuiReady stays false there so
    // shutdown does not tear that context down.
    if (m_ImGuiReady || ImGui::GetCurrentContext() != nullptr) {
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
    if (!m_ImGuiReady && ImGui::GetCurrentContext() == nullptr) {
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

std::size_t UiRuntime::StyleDepth() const {
    return m_StyleStack.size();
}

void UiRuntime::RestoreStyleDepth(std::size_t depth) {
    while (m_StyleStack.size() > depth) {
        PopStyle();
    }
}

void UiRuntime::SetPreview(bool preview, float x, float y, float width, float height) {
    m_Preview = preview;
    m_PreviewRect = {x, y, width, height};
}

bool UiRuntime::IsPreview() const {
    return m_Preview;
}

std::array<float, 4> UiRuntime::GetScreenRect() const {
    if (m_Preview) {
        return m_PreviewRect;
    }
    if (ImGui::GetCurrentContext() == nullptr) {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }
    return {0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y};
}

void UiRuntime::BeginScreen(const std::string& id, int flags) {
    std::array<float, 4> rect = GetScreenRect();

    BeginRect(id, rect[0], rect[1], rect[2], rect[3], flags);
}

void UiRuntime::EndScreen() {
    EndRect();
}

void UiRuntime::BeginPanel(const std::string& id, float width, float height, int flags) {
    std::array<float, 4> rect = GetScreenRect();

    BeginRect(id, rect[0] + (rect[2] - width) * 0.5f, rect[1] + (rect[3] - height) * 0.5f, width,
              height, flags);
}

void UiRuntime::EndPanel() {
    EndRect();
}

void UiRuntime::BeginRect(const std::string& id, float x, float y, float width, float height,
                          int flags) {
    ImGuiWindowFlags windowFlags = static_cast<ImGuiWindowFlags>(flags);

    if (!m_Preview) {
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        ImGui::Begin(id.c_str(), nullptr, windowFlags);
        return;
    }
    // BeginChild asserts on AlwaysAutoResize, the only window flag a child rejects.
    windowFlags &= ~ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::SetCursorScreenPos(ImVec2(x, y));
    // Templates push WindowBg for their page, but a child fills with ChildBg.
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
    ImGui::BeginChild(id.c_str(), ImVec2(width, height), ImGuiChildFlags_None, windowFlags);
}

void UiRuntime::EndRect() {
    if (!m_Preview) {
        ImGui::End();
        return;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
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

    // A script next to the game overrides the copy shipped with the UI library.
    path = ResolveAssetPath(registry, "ui/templates/" + name + ".lua");
    if (!std::filesystem::exists(path)) {
        path = std::filesystem::path(m_TemplateRoot) / (name + ".lua");
    }
    if (!std::filesystem::exists(path)) {
        log_user_error("[UI] Template not found: " + name);
        return lua.create_table();
    }

    result = lua.script_file(path.string());
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
    // Zero means the texture's own size.
    if (drawWidth <= 0) {
        drawWidth = found->second->texture.width;
    }
    if (drawHeight <= 0) {
        drawHeight = found->second->texture.height;
    }
    rlImGuiImageSize(&found->second->texture, drawWidth, drawHeight);
}

} // namespace Engine
