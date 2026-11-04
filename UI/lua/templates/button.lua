-- Returns true on the click that also calls onClick.
local Button = {}

Button.defaults = {
    label = "Button",
    style = "primary",
    width = 280,
    height = 40,
}

function Button.draw(props)
    local p = UI.merge(Button.defaults, props)
    local clicked = false

    UI.pushStyle(p)
    if p.width and p.height then
        clicked = ImGui.Button(p.label, p.width, p.height)
    else
        clicked = ImGui.Button(p.label)
    end
    UI.popStyle()

    if clicked and type(p.onClick) == "function" then
        p.onClick()
    end
    return clicked
end

return Button
