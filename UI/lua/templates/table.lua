-- Cells in a row are 1-based. ImGui columns start at 0.
local Table = {}

Table.defaults = {
    id = "table",
    columns = {},
    rows = {},
    style = "primary",
}

function Table.draw(props)
    local p = UI.merge(Table.defaults, props)
    local columns = p.columns or {}
    local rows = p.rows or {}
    local flags = ImGuiTableFlags.Borders | ImGuiTableFlags.RowBg

    if #columns == 0 then
        return
    end

    UI.pushStyle(p)
    if ImGui.BeginTable(p.id or "table", #columns, flags) then
        for _, name in ipairs(columns) do
            ImGui.TableSetupColumn(name)
        end
        ImGui.TableHeadersRow()
        for _, row in ipairs(rows) do
            ImGui.TableNextRow()
            for column, cell in ipairs(row) do
                ImGui.TableSetColumnIndex(column - 1)
                ImGui.Text(tostring(cell))
            end
        end
        ImGui.EndTable()
    end
    UI.popStyle()
end

return Table
