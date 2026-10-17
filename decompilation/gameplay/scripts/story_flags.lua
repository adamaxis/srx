-- FFFC1E story-progress long and the FFDD64 "active story trigger" rule.
--
-- ROM facts [C]:
--   * 0x53A78 (the field trigger loop) stores the index of the trigger it is
--     dispatching in FFDD64 before calling that trigger's service.
--   * 0x2196C is the one generic setter:
--         MOVE.L FFDD64,D0 ; MOVE.L FFFC1E,D1 ; BSET D0,D1 ; MOVE.L D1,FFFC1E
--     BSET on a data register uses D0 mod 32.
--   * Dialogue command table 0x12CCB4 (dispatched by 0x1F580). Commands
--     0x15..0x1A and 0x1D branch to 0x2196C without touching FFDD64, so they
--     set the bit of whichever trigger opened the conversation. Command 0x05
--     (0x213FA) does the same, but only on its paid branch (0x21410).
--   * Other commands load FFDD64 with a fixed index first, then branch to
--     0x2196C. That write persists, so a later active-trigger command in the
--     same conversation sees the new index (modelled by mark_fixed).
--
-- The host exposes FFFC1E as story_flag_bit_<n> and FFDD64 as
-- story_trigger_active_<n> facts. Both read as nil while the session has not
-- restored them.
local E = require("enums")
local Triggers = require("story_triggers")

local StoryFlags = {}

StoryFlags.BIT_COUNT = 32
local BIT_MASK = StoryFlags.BIT_COUNT - 1

-- Command -> marking behaviour, from table 0x12CCB4 [C].
--   "active" = set bit FFDD64
--   number   = load FFDD64 with that index, then set it
--   table    = the handler picks one of the listed indices on a branch
StoryFlags.DIALOGUE_COMMAND_MARKS = {
    [0x05] = "active_on_paid_branch", -- 0x213FA; bsr 0x2196C at 0x21410
    [0x15] = "active",                -- 0x218B2
    [0x16] = "active",                -- 0x218BC
    [0x17] = "active",                -- 0x218C6 (also 0x59C34 +1)
    [0x18] = "active",                -- 0x218DA
    [0x19] = "active",                -- 0x218E4
    [0x1A] = "active",                -- 0x218F8
    [0x1B] = E.StoryFlag.RITUAL_OF_BINDING,    -- 0x21902: FFDD64 = 0x11
    [0x1C] = E.StoryFlag.MICHAEL_MISSION_TOLD, -- 0x2193A: FFDD64 = 0x0D
    [0x1D] = "active",                -- 0x21962
    [0x37] = E.StoryFlag.GILLIAN_INITIATION,   -- 0x21E08: FFDD64 = 0x14
    [0x40] = E.StoryFlag.POST_ITO_BRIEFED,     -- 0x21F32: FFDD64 = 0x1C
    [0x46] = { E.StoryFlag.RENRAKU_MAP_SHOWN, E.StoryFlag.BOTH_MAPS_SHOWN }, -- 0x22018
    [0x47] = { E.StoryFlag.RENRAKU_MAP_SHOWN, E.StoryFlag.BOTH_MAPS_SHOWN }, -- 0x22072
    [0x61] = E.StoryFlag.HOSPITAL_VISITED,     -- 0x22648: FFDD64 = 0x19
}

local names_by_bit = {}
for name, bit in pairs(E.StoryFlag) do
    names_by_bit[bit] = name
end

local function checked_bit(flag)
    assert(math.type(flag) == "integer" and flag >= 0 and
           flag < StoryFlags.BIT_COUNT,
        "story flag must be an FFFC1E bit index 0..31")
    return flag
end

local function bit_fact(flag)
    return "story_flag_bit_" .. tostring(checked_bit(flag))
end

-- Readable E.StoryFlag name for a bit, or nil when the bit is unnamed.
function StoryFlags.name(flag)
    return names_by_bit[checked_bit(flag)]
end

-- true/false, or nil when FFFC1E is not restored in this session.
function StoryFlags.is_set(flag)
    return game.state[bit_fact(flag)]
end

function StoryFlags.set(flag)
    game.state[bit_fact(flag)] = true
end

-- The FFDD64 trigger index, or nil when no trigger dispatch is recorded.
function StoryFlags.active_trigger()
    for index = 0, BIT_MASK do
        if game.state["story_trigger_active_" .. index] == true then
            return index
        end
    end
    return nil
end

-- 0x2196C: FFFC1E |= 1 << (FFDD64 mod 32). Returns the bit that was set.
function StoryFlags.mark_active_trigger()
    local trigger = StoryFlags.active_trigger()
    assert(trigger ~= nil,
        "0x2196C needs the FFDD64 trigger index from 0x53A78")
    local flag = trigger & BIT_MASK
    StoryFlags.set(flag)
    return flag
end

-- Fixed-index handlers: MOVE.L #index,FFDD64 then 0x2196C.
function StoryFlags.mark_fixed(flag)
    checked_bit(flag)
    game.state["story_trigger_active_" .. flag] = true
    StoryFlags.set(flag)
    return flag
end

-- Apply one dialogue command's 0x2196C effect. `branch` picks the index for
-- two-way handlers (1 or 2), and `paid` gates command 0x05. Returns the bit
-- set, or nil if the command does not reach 0x2196C on this path.
function StoryFlags.apply_dialogue_command(command, options)
    options = options or {}
    local mark = StoryFlags.DIALOGUE_COMMAND_MARKS[command]
    if mark == nil then
        return nil
    elseif mark == "active" then
        return StoryFlags.mark_active_trigger()
    elseif mark == "active_on_paid_branch" then
        if options.paid ~= true then
            return nil
        end
        return StoryFlags.mark_active_trigger()
    elseif type(mark) == "table" then
        local flag = mark[options.branch]
        assert(flag ~= nil, "two-way story command needs branch 1 or 2")
        return StoryFlags.mark_fixed(flag)
    end
    return StoryFlags.mark_fixed(mark)
end

-- 0x53A78: store the dispatched trigger index in FFDD64 before calling its
-- handler. Only live table entries (story_triggers.lua) dispatch anything.
function StoryFlags.begin_trigger(index)
    assert(Triggers.is_live(index),
        "FFDD64 only holds a live 0x15FD5C trigger index")
    game.state["story_trigger_active_" .. index] = true
    return index
end

-- The handler's own jsr 0x2196C after its conversation (triggers 4, 15, 18
-- and 22). It marks the CURRENT FFDD64, which a fixed-index dialogue command
-- may have moved. Returns the bit set, or nil when the handler has no tail.
function StoryFlags.finish_trigger(index)
    local live = Triggers.LIVE[index]
    assert(live ~= nil, "not a live trigger")
    if not live.tail or #live.tail == 0 then return nil end
    return StoryFlags.mark_active_trigger()
end

return StoryFlags
