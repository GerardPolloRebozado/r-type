-- value is 1-based. ImGui's combo index starts at 0.
local Combo = {}

Combo.defaults = {
    label = "Choice",
    value = 1,
    options = {},
    width = 220,
    style = "primary",
}

function Combo.draw(props)
    local p = UI.merge(Combo.defaults, props)
    local options = p.options or {}
    local index = (p.value or 1) - 1
    local value = p.value or 1

    if index < 0 then
        index = 0
    end

    UI.pushStyle(p)
    if p.width then
        ImGui.SetNextItemWidth(p.width)
    end
    index = ImGui.Combo(p.label, index, options, #options)
    UI.popStyle()
    value = index + 1
    return value
end

return Combo
