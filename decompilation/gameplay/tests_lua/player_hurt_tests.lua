-- ROM 0xAFF2: party meter damage queues the player-side C072 response while
-- world actors continue to use their existing incoming_hit event.
local E = require("enums")
local Combat = require("combat")
game = {
    actors = {},
    modules = {
        combat = Combat,
        items = { catalog = {} },
        magic = {},
    },
}
local ActorAI = require("actor.init")
local ActorAnim = require("actor.anim")

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function combatant(record_type)
    return {
        record_type = record_type,
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = false,
                ai_runner = false, downed = false, group_leader = false,
                dead = false },
            interaction = { projectile = false,
                proximity_interaction_pending = false,
                use_collision_heading = false, collision_recovery_active = false,
                action_hook_pending = false, defense_difficulty_reduction = false,
                breathing_out = false, unseen = true,
                no_coverage_difficulty_bonus = true },
        },
        physical_meter = 10,
        mental_meter = 10,
        force = 0,
        posture_reduction_percent = 0,
        attributes = {
            body = 0, quickness = 0, strength = 4,
            intelligence = 0, willpower = 0,
        },
        skills = {
            sorcery = 0, firearms = 0, pistols = 0, smgs = 0,
            shotguns = 0, melee_combat = 1,
        },
        combat_traits = {
            ignores_cover_condition_penalty = true,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
        equipped_action_id = "test_weapon",
        cyberware = {},
        action_profile_options = { primary = false, secondary = false },
        inventory = {},
    }
end

local attacker = combatant(E.ActorPool.PARTY)
attacker.combat_traits.ignores_cover_condition_penalty = true
local bound_calls = 0
local function random_sixes_then_ones(bound)
    if bound ~= 6 then return nil end
    bound_calls = bound_calls + 1
    return bound_calls % 2 == 1 and 5 or 0
end
local function hit(target)
    return Combat.resolve_reaction({
        combatant = attacker,
        combat_meter_service_blocked = false,
        ai_random_bound = random_sixes_then_ones,
    }, target)
end

local party_target = {
    id = "party.runner.1",
    source_record_slot = 1,
    combatant = combatant(1),
}
local party_hit = hit(party_target)
check(party_hit ~= nil and party_hit.changed and
      party_hit.physical_meter > 0 and
      party_hit.mental_meter < party_target.combatant.mental_meter and
      game.player_hurt_pending_by_actor_id[party_target.id] == true,
      "party damage with nonzero physical meter queues player C072")

local world_target = {
    id = "world.actor.3",
    source_record_slot = E.ActorPool.PARTY,
    combatant = combatant(E.ActorPool.PARTY),
}
local world_hit = hit(world_target)
check(world_hit ~= nil and world_hit.changed and
      game.actors[world_target.id].incoming_hit ~= nil and
      game.player_hurt_pending_by_actor_id[party_target.id] == true,
      "world damage keeps the existing incoming_hit path")

local hurt_recipient_id = party_target.id
game.player_hurt_pending_by_actor_id = {[hurt_recipient_id] = true}
local unrelated_actor_state = {actor_id = "world.actor.4"}
local unrelated_actor_context = {
    actor_id = unrelated_actor_state.actor_id,
    controlled_party_member_id = hurt_recipient_id,
    is_controlled_party_actor = false,
    phase = "behavior",
}
local unrelated_actor_started = ActorAnim.begin_player_hurt_reaction(
    unrelated_actor_state, unrelated_actor_context)
check(not unrelated_actor_started and
      game.player_hurt_pending_by_actor_id[hurt_recipient_id] == true and
      unrelated_actor_state.player_hurt_pose_active ~= true,
      "world actor cannot consume another actor's player-hurt event")

local function player_context(phase, spawn)
    return {
        actor_id = hurt_recipient_id,
        profile_id = "field_runner_k03_none00",
        entity_kind = E.SpriteType.FIELD_RUNNER,
        action_kind = "none",
        action_code = 0,
        phase = phase,
        initial_heading = 0,
        entry_delay = 0,
        spawn = spawn,
        source_record_slot = 1,
        controlled_party_member_id = hurt_recipient_id,
        is_controlled_party_actor = true,
        actor_facts = {
            lifecycle = { active = true, thinking_enabled = false,
                ai_runner = false, downed = false, group_leader = true,
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
        },
        movement_allowed = false,
        controller_nibble = 0,
        combat_action_available = true,
        combat_action_ready = true,
        combat_confirm_held = true,
        combat_input_blocked = false,
    }
end

ActorAI.heartbeat(player_context("behavior", true))
local hurt_start = ActorAI.heartbeat(player_context("behavior", false))
check(hurt_start ~= nil and hurt_start.animation == "runner_hurt" and
      hurt_start.combat_action_requested == true and
      game.player_hurt_pending_by_actor_id[hurt_recipient_id] == nil,
      "player consumes hurt event without blocking action input")

local render_context = player_context("render", false)
render_context.animation_mode = "idle"
local hurt_first = ActorAI.heartbeat(render_context)
render_context.animation_mode = "runner_hurt"
local exact_cue = true
for service = 1, E.PlayerHurt.SOUND_DELAY do
    local decision = ActorAI.heartbeat(player_context("behavior", false))
    local wants_cue = service == E.PlayerHurt.SOUND_DELAY
    if (decision ~= nil and decision.audio_event ==
            E.SoundId.DAC_SAMPLE_40) ~= wants_cue then
        exact_cue = false
    end
    hurt_last = ActorAI.heartbeat(render_context)
end
for service = E.PlayerHurt.SOUND_DELAY + 1,
               E.PlayerHurt.RENDER_PASSES - 1 do
    local decision = ActorAI.heartbeat(player_context("behavior", false))
    if decision ~= nil and decision.audio_event ~= nil then
        exact_cue = false
    end
    hurt_last = ActorAI.heartbeat(render_context)
end
local hurt_finished = ActorAI.heartbeat(player_context("behavior", false))
render_context.animation_mode = "idle"
local hurt_recovered = ActorAI.heartbeat(render_context)
check(hurt_first ~= nil and hurt_first.animation == "runner_hurt" and
      hurt_first.frame == 0 and
      hurt_first.animation_selector == nil and
      hurt_first.animation_accumulator == nil and
      hurt_last ~= nil and hurt_last.animation == "runner_hurt" and
      hurt_last.frame == E.PlayerHurt.FRAME_COUNT - 1 and
      hurt_last.animation_accumulator == nil and
      hurt_finished ~= nil and hurt_finished.animation == "idle" and
      hurt_finished.animation_selector == nil and
      hurt_recovered ~= nil and hurt_recovered.animation == "idle" and
      hurt_recovered.frame == 0,
      "player C072 completes after its finite ROM list and returns to C052")
check(exact_cue,
      "B07E emits the kind-3 B03E cue on the eighth later service")

print(string.format("player_hurt_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
