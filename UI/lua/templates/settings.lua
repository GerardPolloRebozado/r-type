local Settings = {}

Settings.defaults = {
    title = "Settings",
    width = 520,
    height = 560,
    rows = {},
    style = "primary",
}

function Settings.draw(props)
    local p = UI.merge(Settings.defaults, props)
    local rows = p.rows or {}
    local values = {}
    local displayW, displayH = ImGui.GetDisplaySize()
    local width = p.width or 520
    local height = p.height or 560

    UI.pushStyle(p)
    ImGui.SetNextWindowPos((displayW - width) * 0.5, (displayH - height) * 0.5)
    ImGui.SetNextWindowSize(width, height)
    ImGui.Begin(p.title or "Settings")
    ImGui.Text(p.title or "Settings")
    ImGui.Separator()
    ImGui.Spacing()

    for index, row in ipairs(rows) do
        local template = UI.use(row.template or "label")
        local rowProps = row.props or {}
        local key = rowProps.label or row.template or index
        if template.draw then
            values[key] = template.draw(rowProps)
        end
        ImGui.Spacing()
    end

    ImGui.End()
    UI.popStyle()
    return values
end

return Settings
