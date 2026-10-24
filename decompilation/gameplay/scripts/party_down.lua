-- Party runner down (ROM 0xB330) and party wipe recovery (0xC64C/0xC716).
-- Record flags are the +0x10 status byte (E.RunnerStatus). Slot 0..2 are
-- FF0110/FF0210/FF0310. The host owns timers, text panels and scene moves.
local E = require("enums")

local PartyDown = {}

local ACTIVE = E.RunnerStatus.ACTIVE  -- 0x01
local DOWN = E.RunnerStatus.DOWN      -- 0x10
local DEAD = E.RunnerStatus.DEAD      -- 0x40

PartyDown.WIPE_COUNTDOWN = 0x3C       -- 0xABF2 FFE166 = 60 frames [C]
PartyDown.DOWN_SELECTOR_BASE = 0xE8   -- 0xC0B2 words E8..104 [C]
PartyDown.DOWN_LIST_INCREMENT = 0x200 -- C0B2 list header 0x8008, +0x200 [C]
PartyDown.LEADER_MIN_PHYSICAL = 6     -- 0xC750 [C]
PartyDown.LEADER_MIN_MENTAL = 8       -- 0xC764 [C]
PartyDown.NUYEN_PENALTY_CAP = 5000    -- 0xC7AA 0x1388 [C]
PartyDown.WAKE = { LITTLE_CHIBA = 0, DOCWAGON = 1 } -- 0xC778 FFFC1E bit0 [C]

-- 0xB05E word table indexed by record +0x0E (kind) [C].
local DOWN_SOUND = { [0] = 0x40, 0x40, 0x40, 0x2E, 0x2F, 0x2E, 0x58, 0x2E }

local function byte(v) return math.tointeger(v or 0) & 0xff end

-- 0xABB6 with the downing record's bit6 set by 0xB330: the leader must be
-- the downing record, and slots 1/2 must be inactive or DEAD [C].
local function all_down(slot, flags)
    local function dead(i)
        local f = flags[i]
        if i == slot then f = f | DEAD end
        if i == 0 then return f & DEAD ~= 0 end
        return f & ACTIVE == 0 or f & DEAD ~= 0
    end
    return dead(0) and dead(1) and dead(2)
end

-- Flat call: slot, flags0, flags1, flags2, kind, heading.
-- Returns { new_flags, sound_id, selector, wipe_countdown }.
function PartyDown.runner_down_flat(slot, f0, f1, f2, kind, heading)
    slot = math.tointeger(slot)
    assert(slot and slot >= 0 and slot <= 2, "party_down: slot 0..2")
    local flags = { [0] = byte(f0), byte(f1), byte(f2) }
    local wipe = all_down(slot, flags) and PartyDown.WIPE_COUNTDOWN or 0
    -- 0xB35C: dir = round(+0x2E / 0x2000) & 7.
    local h = math.tointeger(heading or 0) & 0xffff
    local d = ((h + 0x1000) >> 13) & 7
    return {
        flags[slot] | DOWN,
        DOWN_SOUND[math.tointeger(kind or 0) & 7] or 0,
        PartyDown.DOWN_SELECTOR_BASE + d * 4,
        wipe,
    }
end

-- Flat call: nuyen, story_bit0, then per slot (flags, physical, mental).
-- Returns { leader_flags, leader_physical, leader_mental, dismiss1,
--           dismiss2, nuyen, wake }.
function PartyDown.resolve_wipe_flat(nuyen, story_bit0, f0, p0, m0, f1, _p1,
                                     _m1, f2, _p2, _m2)
    -- 0xC716 clears hired +0x4C first, so 0xC802 dismisses every active
    -- hired runner [C].
    local dismiss1 = byte(f1) & ACTIVE ~= 0 and 1 or 0
    local dismiss2 = byte(f2) & ACTIVE ~= 0 and 1 or 0
    -- 0xC726: leader clears DEAD/DOWN, sets ACTIVE; meters floor at 6/8.
    local leader = (byte(f0) & ~(DEAD | DOWN) | ACTIVE) & 0xff
    local physical = math.max(byte(p0), PartyDown.LEADER_MIN_PHYSICAL)
    local mental = math.max(byte(m0), PartyDown.LEADER_MIN_MENTAL)
    -- 0xC7A2: FFFBFE -= min(FFFBFE >> 3, 5000).
    local funds = math.max(math.tointeger(nuyen or 0), 0)
    funds = funds - math.min(funds >> 3, PartyDown.NUYEN_PENALTY_CAP)
    local wake = (story_bit0 ~= 0 and story_bit0 ~= false)
        and PartyDown.WAKE.DOCWAGON or PartyDown.WAKE.LITTLE_CHIBA
    return { leader, physical, mental, dismiss1, dismiss2, funds, wake }
end

-- Dialogue task: present the 0xC778 wake-up passage. DocWagon (C25A0) has
-- two records, Little Chiba (C25BC) one [C].
function PartyDown.present_wake()
    local StoryFlags = require("story_flags")
    if StoryFlags.is_set(E.StoryFlag.MOTEL_BILL_PAID) then
        game.ui.say("party_wipe_docwagon")
        game.ui.say("party_wipe_docwagon_dropoff")
        return "party_wipe_docwagon"
    end
    game.ui.say("party_wipe_little_chiba")
    return "party_wipe_little_chiba"
end

return PartyDown
