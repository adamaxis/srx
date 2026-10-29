-- ROM 0xB0AC / 0xB0E4 spell deaths (begin_received_hit_reaction).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local E = require("enums")
local K = E.SpriteType
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end
local function hit(kind, lifecycle, variant)
    local st = { entity_kind = kind, actor_facts = {
            lifecycle = { active = lifecycle.active == true,
                ai_runner = lifecycle.ai_runner == true, downed = false },
        }, direction = 2,
        incoming_hit = { physical_meter = 0, mental_meter = 3,
            spell_death_variant = variant } }
    local cue = R.begin_received_hit_reaction(st, { entity_kind = kind })
    return st, cue
end
local st, cue = hit(5, { active = true }, "spell_death_e0")
check(st.spell_death and st.animation_id == "down" and
      st.spell_death_variant == "spell_death_e0",
      "B0AC selects the semantic E0 death clip [C: 0xB0AC]")
check(not st.actor_facts.lifecycle.downed and cue == nil, "B0AC sets no DOWN and no cue")
st, cue = hit(5, { active = true }, "spell_death_e4")
check(st.spell_death and st.animation_id == "down" and
      st.spell_death_variant == "spell_death_e4",
      "B0E4 selects the semantic E4 death clip [C: 0xB0E4]")
st, cue = hit(K.HELL_HOUND, { active = true }, "spell_death_e0")
check(not st.spell_death and st.actor_facts.lifecycle.downed and cue == R.Animator.down_audio(K.HELL_HOUND), "kind 0xC goes to B330")
st, cue = hit(5, { active = true, ai_runner = true }, "spell_death_e0")
check(not st.spell_death and st.actor_facts.lifecycle.downed and
      cue == R.Animator.down_audio(5), "+0x10 b3 goes to B330")
st, cue = hit(5, { active = true }, nil)
check(not st.spell_death and cue == R.Animator.down_audio(5),
      "plain zero meter is AFF2 -> B330")
print(string.format("spell_death tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
