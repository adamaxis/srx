-- Command overview return timing from the source field-entry palette handoff.
-- ROM 0xB62 calls menu loop 0x10D30; its field return reaches 0x2BD2 ->
-- 0x131C. That routine repeats 0x1088 CRAM decrements after the 0x1342
-- frame wait. Seven visible two-level decreases take bright Genesis channels
-- to black; emulator routes e3/e4 establish the root-close offsets and the
-- field reload hold after that palette work [C].
local E = require("enums")

local M = {}
local Phase = E.FieldReturnTransitionPhase
local Timing = E.FieldReturnTransitionTiming

function M.new()
    return {
        active = true,
        elapsed_frames = 0,
        phase = Phase.MENU_FADE_OUT,
        palette_step = 0,
    }
end

function M.advance(state)
    if not state.active then return state end

    state.elapsed_frames = state.elapsed_frames + 1
    local frame = state.elapsed_frames
    if frame < Timing.FADE_OUT_FIRST_FRAME then
        state.phase = Phase.MENU_FADE_OUT
        state.palette_step = 0
    elseif frame <= Timing.FADE_OUT_LAST_FRAME then
        state.phase = Phase.MENU_FADE_OUT
        state.palette_step = (frame - 1) // 2
    elseif frame <= Timing.BLACK_HOLD_LAST_FRAME then
        state.phase = Phase.BLACK_HOLD
        state.palette_step = Timing.PALETTE_STEPS
    elseif frame < Timing.FIELD_LIVE_FRAME then
        state.phase = Phase.FIELD_FADE_IN
        state.palette_step = (frame - 98) // 2
    else
        state.active = false
        state.phase = Phase.COMPLETE
        state.palette_step = 0
    end
    return state
end

return M
