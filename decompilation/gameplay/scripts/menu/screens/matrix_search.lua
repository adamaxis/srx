-- ROM 0xE1E6/0xE204: draw C19B0 "----.---.----" at VDP C796, then 11
-- permutation rounds (d7 = 10..0). Each round plays sound 0x36, then rewrites
-- ONE code cell d6+1 times (d6 = rand(11)+10) with a random digit, two VBlanks
-- per digit; the last digit stays and the cursor moves one cell right (A2+2),
-- skipping the '.' separators after rounds d7 = 7 and d7 = 4. Finally one
-- selection from the 14 system IDs at C1968.
local E = require("enums")
local Passcodes = require("data.matrix_passcodes")

local M = { window = E.WindowPlane.OFF }
local P, R, T = E.Pad, E.MenuRegion, E.MenuTransition
local ROUNDS = 11
local FRAMES_PER_DIGIT = 2
-- ROM 0xC19B0 placeholder string; '-' cells receive digits, '.' are fixed.
local CODE_TEMPLATE = "----.---.----"

local function random_below(ctx, bound)
    return ctx.game.source_random_bound(bound)
end

local function update_text(st)
    st.title = "System Search"
    st.text = "SYSTEM SEARCH IN PROGRESS...|RUNNING PERMUTATIONS|" ..
              st.code
    st.labels = {}
end

local function publish_digit(st, ctx)
    local digit = random_below(ctx, 10)
    st.digits = string.char(string.byte("0") + digit)
    st.code = st.code:sub(1, st.cell - 1) .. st.digits .. st.code:sub(st.cell + 1)
    st.digits_remaining = st.digits_remaining - 1
    st.frames_remaining = FRAMES_PER_DIGIT
    update_text(st)
    ctx.redraw(R.FULL)
end

local function begin_round(st, ctx)
    if st.round > 0 then
        -- 0xE296..0xE2AA: next cell, plus the separator after d7 = 7 / 4.
        st.cell = st.cell + 1
        if st.code:sub(st.cell, st.cell) == "." then st.cell = st.cell + 1 end
    end
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
    ctx.sound(E.SoundId.MATRIX_SYSTEM_SEARCH) -- 0xE23E, once per round
    -- 0xE248: d6 = rand(11)+10 drives a dbra loop, so d6+1 digits.
    st.digits_remaining = random_below(ctx, 11) + 11
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
        code = CODE_TEMPLATE,
        cell = 1,
        title = "System Search",
        text = "",
        labels = {},
    }
end

function M.enter(st, ctx)
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
