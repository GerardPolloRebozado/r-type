-- Returns the edited string.
local Input = {}

Input.defaults = {
    label = "Text",
    value = "",
    width = 220,
    style = "primary",
}

function Input.draw(props)
    local p = UI.merge(Input.defaults, props)
    local value = p.value or ""

    UI.pushStyle(p)
    if p.width then
        ImGui.SetNextItemWidth(p.width)
    end
    value = ImGui.InputText(p.label, value)
    UI.popStyle()
    return value
end

return Input
