-- Returns the checked state. A missing value counts as unchecked.
local Checkbox = {}

Checkbox.defaults = {
    label = "Option",
    value = false,
    style = "primary",
}

function Checkbox.draw(props)
    local p = UI.merge(Checkbox.defaults, props)
    local value = p.value and true or false

    UI.pushStyle(p)
    value = ImGui.Checkbox(p.label, value)
    UI.popStyle()
    return value
end

return Checkbox
