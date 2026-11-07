-- value is 1-based, the same index ipairs gives each option.
local RadioGroup = {}

RadioGroup.defaults = {
    label = "Choice",
    value = 1,
    options = {},
    style = "primary",
}

function RadioGroup.draw(props)
    local p = UI.merge(RadioGroup.defaults, props)
    local value = math.floor(p.value or 1)
    local options = p.options or {}

    UI.pushStyle(p)
    if p.label and p.label ~= "" then
        ImGui.Text(p.label)
    end
    for index, option in ipairs(options) do
        value = ImGui.RadioButton(option, value, math.floor(index))
    end
    UI.popStyle()
    return value
end

return RadioGroup
