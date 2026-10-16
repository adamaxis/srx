-- Pocket Secretary frame effect, ROM 0xE660.
-- Lua owns the per-screen animation state and cadence. The host only supplies
-- the ROM bounded-random service; the presenter consumes these named cells.
local M = { cells = {}, frames = 0 }

local function clear_cells()
    for index = 1, 12 do M.cells[index] = -1 end
end

clear_cells()

function M.reset()
    clear_cells()
    M.frames = 0
end

function M.advance(ctx)
    local primary = ctx.game.source_random_bound(2)
    local location
    local variant
    if primary == 0 then
        location = ctx.game.source_random_bound(8)
        variant = ctx.game.source_random_bound(3)
    else
        location = 8 + ctx.game.source_random_bound(4)
        variant = 3 + ctx.game.source_random_bound(4)
    end

    if location >= 0 and location < 12 then
        M.cells[location + 1] = variant
    end
    M.frames = M.frames + 1
    return M.frames
end

return M
