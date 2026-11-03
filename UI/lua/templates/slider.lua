local Slider = {}

Slider.defaults = {
    label = "Value",
    value = 0.5,
    min = 0,
    max = 1,
    showValue = true,
    width = 220,
    style = "primary",
}

function Slider.draw(props)
    local p = UI.merge(Slider.defaults, props)
    local value = p.value or 0

    UI.pushStyle(p)
    ImGui.AlignTextToFramePadding()
    ImGui.Text(p.label)
    ImGui.SameLine()
    if p.width then
        ImGui.SetNextItemWidth(p.width)
    end
    value = ImGui.SliderFloat("##" .. p.label, value, p.min or 0, p.max or 1)
    if p.showValue ~= false then
        ImGui.SameLine()
        ImGui.Text(string.format("%.2f", value))
    end
    UI.popStyle()
    return value
end

return Slider
