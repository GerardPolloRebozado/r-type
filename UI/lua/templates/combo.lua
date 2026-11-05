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
    local index = math.floor(p.value or 1) - 1
    local value = math.floor(p.value or 1)

    -- ImGui has nothing to pick from, so the value goes back unchanged.
    if #options == 0 then
        return value
    end
    if index < 0 then
        index = 0
    end

    UI.pushStyle(p)
    if p.width then
        ImGui.SetNextItemWidth(p.width)
    end
    index = ImGui.Combo(p.label, index, options, math.floor(#options))
    UI.popStyle()
    value = index + 1
    return value
end

return Combo
