local Progress = {}

Progress.defaults = {
    label = "",
    value = 0,
    width = 280,
    height = 22,
    style = "primary",
}

function Progress.draw(props)
    local p = UI.merge(Progress.defaults, props)
    local value = p.value or 0

    if value < 0 then
        value = 0
    elseif value > 1 then
        value = 1
    end

    UI.pushStyle(p)
    ImGui.ProgressBar(value, p.width or 280, p.height or 22, p.label or "")
    UI.popStyle()
    return value
end

return Progress
