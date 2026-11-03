local Menu = {}

Menu.defaults = {
    title = "Menu",
    width = 420,
    height = 480,
    items = {},
    style = "primary",
}

local function openPanel(title, width, height)
    local displayW, displayH = ImGui.GetDisplaySize()
    ImGui.SetNextWindowPos((displayW - width) * 0.5, (displayH - height) * 0.5)
    ImGui.SetNextWindowSize(width, height)
    ImGui.Begin(title)
end

function Menu.draw(props)
    local p = UI.merge(Menu.defaults, props)
    local Button = UI.use("button")
    local items = p.items or {}

    UI.pushStyle(p)
    openPanel(p.title or "Menu", p.width or 420, p.height or 480)
    ImGui.Text(p.title or "Menu")
    ImGui.Separator()
    ImGui.Spacing()
    for _, item in ipairs(items) do
        Button.draw({
            label = item.label or "Item",
            style = item.style or "primary",
            width = (p.width or 420) - 48,
            height = 42,
            onClick = item.onClick,
            enabled = item.enabled,
        })
    end
    ImGui.End()
    UI.popStyle()
end

return Menu
