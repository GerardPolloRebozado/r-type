-- input.lua (local override)
-- Returns the edited string and stores it in _G for multiplayer host/port config.

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

    -- Store the value in _G so multiplayer.lua can read it via setHost/setPort
    local label = p.label or ""
    if label == "Host:" then
        _G.MP_HOST = value
    elseif label == "Port:" then
        local pnum = tonumber(value)
        if pnum and pnum > 0 and pnum <= 65535 then
            _G.MP_PORT = pnum
        end
    end

    return value
end

return Input
