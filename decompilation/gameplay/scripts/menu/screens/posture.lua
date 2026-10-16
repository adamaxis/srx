-- posture.lua: posture focus 0x119CE..0x11AE2 (blocking loop in the ROM, a route screen here).
local E = require("enums")
local P, R, T, S = E.Pad, E.MenuRegion, E.MenuTransition, E.MenuSound
local M = { window = E.WindowPlane.SIDEBAR }

function M.new() return {} end

local BARS = R.POSTURE_BLOCK | R.COMBAT_BARS        -- 0x11ABE: strip + 0x12CEE twice [C]

function M.enter(st, ctx) ctx.redraw(R.POSTURE_LABEL); ctx.redraw(BARS) end   -- focused label 0xCD514 [C]

-- 0x11AE4 adjust, clamp 0..4 [C]
function M.adjust(ctx, d)
    local v = ctx.game.leader_posture() + d
    if v < E.Posture.FULL_DEFENSE then v = E.Posture.FULL_DEFENSE end
    if v > E.Posture.FULL_OFFENSE then v = E.Posture.FULL_OFFENSE end
    ctx.game.set_leader_posture(v)
end

local function close(ctx, exit_menu)               -- 0x11A9A: label 0xCD508, delta 0, bars [C]
    ctx.redraw(R.POSTURE_LABEL); M.adjust(ctx, 0); ctx.redraw(BARS)
    return { transition = T.POP, result = { exit_menu = exit_menu } }
end

function M.input(st, events, ctx)
    local check_right = true
    if events & P.DOWN ~= 0 then
        -- Down edge: held Right -> right test; held Left -> left test; else close (FFF115 is always armed here) [C]
        if events & P.HELD_RIGHT ~= 0 then check_right = true
        elseif events & P.HELD_LEFT ~= 0 then check_right = false
        else return close(ctx, false) end
    end
    local d = 0
    if check_right and events & P.RIGHT ~= 0 then d = 1
    elseif events & P.LEFT ~= 0 then d = -1 end
    if d ~= 0 then ctx.sound(S.MOVE); M.adjust(ctx, d); ctx.redraw(BARS); return end
    -- only reached when no L/R edge fired; Start sets FFF0B1 -> the command loop exits too [C]
    if events & P.START ~= 0 then return close(ctx, true) end
end

return M
