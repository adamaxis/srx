-- ROM 0xB0AC / 0xB0E4 spell deaths (begin_received_hit_reaction).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local E = require("enums")
local S, K = E.ActorStatus, E.SpriteType
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end
local function hit(kind, flags, pose)
    local st = { source_record_flags = flags, direction = 2,
        incoming_hit = { physical_meter = 0, mental_meter = 3, spell_death_pose = pose } }
    local cue = R.begin_received_hit_reaction(st,
        { entity_kind = kind, entry_animation_frame_count = 5, down_audio_event = 0x56 })
    return st, cue
end
local st, cue = hit(5, S.ACTIVE, 0xE0)
check(st.spell_death and st.animation_selector == 0xE0, "B0AC selects pose 0xE0")
check((st.source_record_flags & S.DOWN) == 0 and cue == nil, "B0AC sets no DOWN and no cue")
st, cue = hit(5, S.ACTIVE, 0xE4)
check(st.spell_death and st.animation_selector == 0xE4, "B0E4 selects pose 0xE4")
st, cue = hit(K.HELL_HOUND, S.ACTIVE, 0xE0)
check(not st.spell_death and (st.source_record_flags & S.DOWN) ~= 0 and cue == 0x56, "kind 0xC goes to B330")
st, cue = hit(5, S.ACTIVE | S.AI_RUNNER, 0xE0)
check(not st.spell_death and (st.source_record_flags & S.DOWN) ~= 0, "+0x10 b3 goes to B330")
st, cue = hit(5, S.ACTIVE, nil)
check(not st.spell_death and cue == 0x56, "plain zero meter is AFF2 -> B330")
print(string.format("spell_death tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
