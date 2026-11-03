local Image = {}

Image.defaults = {
    path = "",
    width = 128,
    height = 128,
}

function Image.draw(props)
    local p = UI.merge(Image.defaults, props)

    if not p.path or p.path == "" then
        return
    end
    UI.image(p.path, p.width or 128, p.height or 128)
end

return Image
