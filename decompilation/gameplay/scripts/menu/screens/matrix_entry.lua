-- Field Matrix entry shared by known passcodes and System Search.
-- ROM 0xE47A plays 0x1A, waits 20 VBlanks, then calls 0x181F8.
local E = require("enums")
local MatrixSession = require("menu.matrix_session")

local M = { window = E.WindowPlane.OFF }
local R, T = E.MenuRegion, E.MenuTransition

function M.new(args)
    args = args or {}
    return {
        system_id = args.system_id,
        return_to_field = args.return_to_field == true,
        wait_frames = 20,
        title = "",
        text = "",
        labels = {},
    }
end

function M.enter(st, ctx)
    ctx.sound(E.MenuSound.JACK_IN)
    ctx.redraw(R.FULL)
end

function M.input(st, events, ctx)
    if st.wait_frames > 1 then
        st.wait_frames = st.wait_frames - 1
        return
    end

    local world = MatrixSession.world_state(ctx.game)
    local system_id = st.system_id
    if system_id == 0xFF then
        system_id = world.run and world.run.target_system_id or nil
    end
    if type(system_id) ~= "number" or system_id % 1 ~= 0 or
       system_id < 0 or system_id > 24 then
        -- A passcode may select FFFBBC only when its current run supplies the
        -- target. Keep an unresolved target out of the Matrix host entirely.
        return { transition = T.EXIT, result = { target_unresolved = true } }
    end

    local started, matrix_state = MatrixSession.begin(
        system_id, world.field and world.field.alarm)
    if not started then
        return { transition = T.EXIT, result = { system_unavailable = true } }
    end
    if world.run and MatrixSession.activate_run_for_system(
        system_id, world.run) then
        MatrixSession.commit_world_state(ctx.game, world)
    end
    return {
        transition = T.REPLACE,
        screen = E.MenuScreen.MATRIX_NODE_ACTION,
        args = {
            system_id = system_id,
            matrix_session = true,
            matrix_state = matrix_state,
            return_to_field = st.return_to_field,
        },
    }
end

return M
