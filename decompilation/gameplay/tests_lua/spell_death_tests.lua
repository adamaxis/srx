-- ROM 0xB0AC / 0xB0E4 spell deaths (begin_received_hit_reaction).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local E = require("enums")
local K = E.ActorVisualProfile
local Combat = require("combat")
game.modules.combat = Combat
game.modules.magic = require("magic")
game.modules.items = require("items")
local pass, total = 0, 0
local function check(c, m) total = total + 1; if c then pass = pass + 1 else print("FAIL: " .. m) end end
local function hit(kind, lifecycle, variant)
    local st = { visual_profile_id = kind, actor_facts = {
            lifecycle = { active = lifecycle.active == true,
                ai_runner = lifecycle.ai_runner == true, downed = false },
        }, direction = 2,
        incoming_hit = { physical_meter = 0, mental_meter = 3,
            spell_death_variant = variant } }
    local cue = R.begin_received_hit_reaction(st, { visual_profile_id = kind })
    return st, cue
end
local st, cue = hit(K.HUMAN_GUARD, { active = true }, "spell_death_e0")
check(st.spell_death and st.animation_id == "down" and
      st.spell_death_variant == "spell_death_e0",
      "B0AC selects the semantic E0 death clip [C: 0xB0AC]")
check(not st.actor_facts.lifecycle.downed and cue == nil, "B0AC sets no DOWN and no cue")
st, cue = hit(K.HUMAN_GUARD, { active = true }, "spell_death_e4")
check(st.spell_death and st.animation_id == "down" and
      st.spell_death_variant == "spell_death_e4",
      "B0E4 selects the semantic E4 death clip [C: 0xB0E4]")
st, cue = hit(K.HELL_HOUND, { active = true }, "spell_death_e0")
check(not st.spell_death and st.actor_facts.lifecycle.downed and cue == R.Animator.down_audio(K.HELL_HOUND), "kind 0xC goes to B330")
st, cue = hit(K.HUMAN_GUARD,
    { active = true, ai_runner = true }, "spell_death_e0")
check(not st.spell_death and st.actor_facts.lifecycle.downed and
      cue == R.Animator.down_audio(K.HUMAN_GUARD), "+0x10 b3 goes to B330")
st, cue = hit(K.HUMAN_GUARD, { active = true }, nil)
check(not st.spell_death and cue == R.Animator.down_audio(K.HUMAN_GUARD),
      "plain zero meter is AFF2 -> B330")

local function lethal_flame_bolt(blocked_pose)
    game.actors = {}
    local target_id = blocked_pose and "thon" or "ordinary_actor"
    local combatant = {
        is_party_member = false,
        blocks_spell_death_pose = blocked_pose,
        physical_meter = 1,
        mental_meter = 10,
        attributes = { body = 1, willpower = 1 },
        combat_traits = {
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
        },
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = false,
                ai_runner = false, downed = false, group_leader = false,
                dead = false },
        },
        inventory = {},
    }
    local result = Combat.resolve_spell_impact({
        combatant = {
            is_party_member = true,
            force = 0,
            inventory = {},
            actor_facts = { interaction = {
                defense_difficulty_reduction = false,
            } },
        },
        ai_random_bound = function(bound)
            if bound ~= 6 then return nil end
            return 4
        end,
    }, {
        spell_id = "flame_bolt",
        action_result = 10,
        target_id = target_id,
    }, {
        id = target_id,
        combatant = combatant,
    })
    return result, game.actors[target_id]
end

local ordinary_result, ordinary_state = lethal_flame_bolt(false)
check(ordinary_result ~= nil and ordinary_result.record_type == nil and
      ordinary_state ~= nil and ordinary_state.incoming_hit ~= nil and
      ordinary_state.incoming_hit.spell_death_variant == "spell_death_e0",
      "flame-bolt result relinks by actor ID and keeps its death pose")
local thon_result, thon_state = lethal_flame_bolt(true)
check(thon_result ~= nil and thon_result.record_type == nil and
      thon_state ~= nil and thon_state.incoming_hit ~= nil and
      thon_state.incoming_hit.spell_death_variant == nil,
      "kind-15 Thon suppresses the flame-bolt death pose by named host fact")

print(string.format("spell_death tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
