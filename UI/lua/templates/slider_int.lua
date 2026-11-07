-- Returns the new integer. "##" keeps the visible label off the slider id.
local SliderInt = {}

SliderInt.defaults = {
    label = "Count",
    value = 1,
    min = 0,
    max = 10,
    width = 220,
    style = "primary",
}

function SliderInt.draw(props)
    local p = UI.merge(SliderInt.defaults, props)
    local value = math.floor(p.value or 0)

    UI.pushStyle(p)
    ImGui.AlignTextToFramePadding()
    ImGui.Text(p.label)
    ImGui.SameLine()
    if p.width then
        ImGui.SetNextItemWidth(p.width)
    end
    value = ImGui.SliderInt("##" .. p.label, value, math.floor(p.min or 0), math.floor(p.max or 10))
    UI.popStyle()
    return value
end

return SliderInt
