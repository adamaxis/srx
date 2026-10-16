-- ROM 0xE1E6: 11 permutation rounds, 10..20 random digits per round,
-- two VBlanks per displayed digit, then one selection from the 14 system IDs.
local E = require("enums")
local Passcodes = require("data.matrix_passcodes")

local M = { window = E.WindowPlane.OFF }
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local ROUNDS = 11
local FRAMES_PER_DIGIT = 2

local function random_below(ctx, bound)
    return ctx.game.source_random_bound(bound)
end

local function update_text(st)
    st.title = "System Search"
    st.text = "SYSTEM SEARCH IN PROGRESS...|RUNNING PERMUTATIONS|" ..
              st.digits
    st.labels = {}
end

local function publish_digit(st, ctx)
    local digit = random_below(ctx, 10)
    st.digits = st.digits .. string.char(string.byte("0") + digit)
    st.digits_remaining = st.digits_remaining - 1
    st.frames_remaining = FRAMES_PER_DIGIT
    update_text(st)
    ctx.redraw(R.FULL)
end

local function begin_round(st, ctx)
    st.round = st.round + 1
    if st.round > ROUNDS then
        local selection = random_below(ctx, #Passcodes.search_system_ids)
        local system_id = Passcodes.search_system_ids[selection + 1]
        return {
            transition = T.REPLACE,
            screen = E.MenuScreen.MATRIX_ENTRY,
            args = {
                system_id = system_id,
                return_to_field = st.return_to_field,
            },
        }
    end
    st.digits_remaining = random_below(ctx, 11) + 10
    st.digits = ""
    publish_digit(st, ctx)
end

function M.new(args)
    args = args or {}
    return {
        return_to_field = args.return_to_field == true,
        round = 0,
        digits_remaining = 0,
        frames_remaining = 0,
        digits = "",
        title = "System Search",
        text = "",
        labels = {},
    }
end

function M.enter(st, ctx)
    ctx.sound(E.SoundId.MATRIX_SYSTEM_SEARCH)
    begin_round(st, ctx)
end

function M.input(st, events, ctx)
    st.frames_remaining = st.frames_remaining - 1
    if st.frames_remaining > 0 then return end

    if st.digits_remaining > 0 then
        publish_digit(st, ctx)
        return
    end

    return begin_round(st, ctx)
end

return M
