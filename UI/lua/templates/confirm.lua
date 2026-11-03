local Confirm = {}

Confirm.defaults = {
    title = "Confirm",
    message = "Are you sure?",
    confirmLabel = "Confirm",
    cancelLabel = "Cancel",
    width = 360,
    height = 180,
    style = "primary",
}

function Confirm.draw(props)
    local p = UI.merge(Confirm.defaults, props)
    local Button = UI.use("button")
    local choice = nil
    local displayW, displayH = ImGui.GetDisplaySize()
    local width = p.width or 360
    local height = p.height or 180

    UI.pushStyle(p)
    ImGui.SetNextWindowPos((displayW - width) * 0.5, (displayH - height) * 0.5)
    ImGui.SetNextWindowSize(width, height)
    ImGui.Begin(p.title or "Confirm")
    ImGui.Text(p.message or "")
    ImGui.Spacing()
    if Button.draw({ label = p.confirmLabel or "Confirm", style = "danger", width = 140, height = 36 }) then
        choice = "confirm"
    end
    ImGui.SameLine()
    if Button.draw({ label = p.cancelLabel or "Cancel", style = "ghost", width = 140, height = 36 }) then
        choice = "cancel"
    end
    ImGui.End()
    UI.popStyle()
    return choice
end

return Confirm
