-- ROM 0x196F2 rotates Cyberdeck CRAM line 1 entries 8-11. FFF10A starts
-- at zero, so the first live deck frame advances immediately, then every
-- five calls. Keep this transient display phase in Lua with the menu state.
local M = {}

function M.new()
    return { countdown = 0, phase = 0 }
end

function M.phase(state)
    return state and state.phase or 0
end

function M.tick(state)
    state.countdown = state.countdown - 1
    if state.countdown < 0 then
        state.countdown = 4
        state.phase = (state.phase + 1) % 4
        return state.phase, true
    end
    return state.phase, false
end

return M
