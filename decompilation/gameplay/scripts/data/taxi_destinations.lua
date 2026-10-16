-- ROM 0xD4456 maps a current field region to its cab-map cursor position.
-- ROM 0xD445D maps that cursor position back to the destination region.
local M = {}

M.cursor_to_region = { 6, 4, 2, 5, 0, 3, 1 }
M.region_to_cursor = { 4, 6, 2, 5, 1, 3, 0 }
M.region_names = {
    [0] = "Downtown Seattle",
    [1] = "Puyallup Barrens",
    [2] = "Redmond Barrens",
    [3] = "Penumbra District",
    [4] = "Council Island",
    [5] = "Renraku Arcology",
    [6] = "Salish-Shidhe",
}

return M
