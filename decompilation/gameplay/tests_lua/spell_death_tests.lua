-- ROM 0xB0AC / 0xB0E4 spell deaths (begin_received_hit_reaction).
local ROOT = os.getenv("SHADOWRUN_ROOT") or "."
local H = dofile(ROOT .. "/decompilation/gameplay/tests_lua/harness.lua")
_G.game = H.make_game({}); require("actor.init")
local R = require("actor.registry")
local E = require("enums")
local ActorAI = require("actor.heartbeat")
local Animator = require("actor.animator")
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
        successes = 10,
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

local function spell_death_lifecycle(variant)
    game.actors = {}
    local actor_id = "static-guard-" .. variant
    local facts = {
        lifecycle = { active = true, thinking_enabled = true,
            ai_runner = false, downed = false, group_leader = false,
            dead = false },
        interaction = { projectile = false,
            proximity_interaction_pending = false,
            use_collision_heading = false, collision_recovery_active = false,
            action_hook_pending = false, defense_difficulty_reduction = false,
            breathing_out = false, unseen = false,
            no_coverage_difficulty_bonus = false },
        behavior = { route_clear = false, path_active = false,
            restart_steering = false, wall_following = false,
            in_target_band = false, route_transition = false,
            fleeing = false, escort = false },
        roles = { spotted = false, reinforcement = false,
            despawn_offscreen = false, quest = false, runner = false,
            pedestrian = false, hostile = false },
    }
    local combatant = {
        is_party_member = false, physical_meter = 1, mental_meter = 10,
        attributes = { body = 5, quickness = 4, strength = 4,
            intelligence = 2, willpower = 2 },
        skills = { firearms = 2, pistols = 0, smgs = 0, shotguns = 0,
            melee_combat = 0, sorcery = 0 },
        combat_traits = { ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false, takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = false,
            is_ghoul_bounty_target = false },
        actor_facts = facts, inventory = {},
    }
    local context = H.context(actor_id)
    context.actor_id = actor_id
    context.profile_id = "corp_guard_k05_item06"
    context.dynamic_world_encounter_actor = false
    context.source_default_item_action_profile = true
    context.visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD
    context.action_kind = "item"
    context.action_code = 6
    context.action_item_id = "max_power"
    context.action_spell_id = nil
    context.actor_facts = facts
    context.combatant = combatant
    context.phase = "behavior"
    context.spawn = true
    context.initial_bearing_degrees = 0
    context.entry_delay = 0
    context.entry_animation_frame_count = 1
    context.live_field_actor = true
    context.source_record_slot = 5
    context.source_projection_active = true
    context.source_spawn_state_flags = 0
    context.source_interaction_flags = 0
    context.source_item_spawn_result = nil
    context.source_think_suppressed = false
    context.world_activity_active = false
    context.world_scene_state = 0
    context.movement_allowed = false
    context.movement_services_available = false
    context.ai_target_candidates = {}
    context.world_target_selection_candidates = {}
    context.combat_meter_service_blocked = false
    context.ai_random_bound = function() return 0 end

    local spawned = ActorAI.heartbeat(context)
    local state = game.actors[actor_id]
    if spawned == nil or state == nil then
        return false, "guard spawn failed"
    end
    state.combatant.physical_meter = 0
    state.physical_condition = 0
    state.incoming_hit = { physical_meter = 0, mental_meter = 3,
        spell_death_variant = variant }
    context.spawn = false
    context.actor_facts = state.actor_facts
    context.combatant = state.combatant
    context.physical_condition = 0
    local down = ActorAI.heartbeat(context)
    if down == nil or state.animation_id ~= "down" or
       state.actor_facts.lifecycle.active ~= true or
       state.actor_facts.lifecycle.downed == true then
        return false, "spell-death pose did not start with ACTIVE and without DOWN"
    end

    local clip_id = variant == "spell_death_e0"
        and Animator.Clip.SPELL_DEATH_E0 or Animator.Clip.SPELL_DEATH_E4
    local clip = Animator.list(state.visual_profile_id, clip_id, state.direction)
    if type(clip) ~= "table" or clip.frame_count < 1 or
       clip.frame_duration_updates < 1 then
        return false, "spell-death clip timing is unavailable"
    end
    context.phase = "render"
    for _ = 1, clip.frame_count * clip.frame_duration_updates + 1 do
        context.actor_facts = state.actor_facts
        context.combatant = state.combatant
        if ActorAI.heartbeat(context) == nil then
            return false, "spell-death render heartbeat failed"
        end
    end
    context.phase = "behavior"
    context.actor_facts = state.actor_facts
    context.combatant = state.combatant
    local completed = ActorAI.heartbeat(context)
    return completed ~= nil and state.actor_facts.lifecycle.dead == true and
           state.actor_facts.lifecycle.active == false and
           state.actor_facts.lifecycle.downed == false,
        completed == nil and "completion heartbeat failed" or
        "0x1E514 did not clear ACTIVE after the spell-death list"
end

for _, variant in ipairs({ "spell_death_e0", "spell_death_e4" }) do
    local retired, reason = spell_death_lifecycle(variant)
    check(retired, variant .. " completes the final list then clears ACTIVE [C: 0xA9A6/0x1E514]: " .. reason)
end

print(string.format("spell_death tests: %d/%d passed", pass, total))
if pass ~= total then os.exit(1) end
