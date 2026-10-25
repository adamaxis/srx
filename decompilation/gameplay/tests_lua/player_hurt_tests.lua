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

local passed, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then passed = passed + 1 else print("FAIL " .. label) end
end

local function combatant(record_type)
    local item_codes, item_phases = {}, {}
    for slot = 1, 8 do
        item_codes[slot] = 0
        item_phases[slot] = 0
    end
    return {
        record_type = record_type,
        source_flags = 1, -- active meter service, actor +0x10 bit 0 [C]
        physical_meter = 10,
        mental_meter = 10,
        force = 0,
        posture = 0,
        command_attribute_78 = 0,
        command_attribute_7b = 0,
        command_attribute_7c = 0,
        command_attribute_77 = 0,
        command_attribute_79 = 4,
        command_attribute_85 = 1,
        race_or_state = 4,
        class_code = 0,
        armor_index = 0,
        equipped_action_code = 1,
        interaction_flags = 0x80,
        cyberware_flags = 0,
        action_profile_modifier_flags = 0,
        item_codes = item_codes,
        item_phases = item_phases,
    }
end

local attacker = combatant(E.ActorPool.PARTY)
attacker.race_or_state = 4
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
      game.player_hurt_pending_by_slot[1] == true,
      "party damage with nonzero physical meter queues player C072")

local world_target = {
    id = "world.actor.3",
    source_record_slot = E.ActorPool.PARTY,
    combatant = combatant(E.ActorPool.PARTY),
}
local world_hit = hit(world_target)
check(world_hit ~= nil and world_hit.changed and
      game.actors[world_target.id].incoming_hit ~= nil and
      game.player_hurt_pending_by_slot[E.ActorPool.PARTY] == nil,
      "world damage keeps the existing incoming_hit path")

local function player_context(phase, spawn)
    return {
        actor_id = "field_player_runner",
        profile_id = "field_runner_k03_none00",
        entity_kind = E.SpriteType.FIELD_RUNNER,
        action_kind = "none",
        action_code = 0,
        phase = phase,
        initial_heading = 0,
        entry_delay = 0,
        spawn = spawn,
        source_record_slot = 1,
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
      game.player_hurt_pending_by_slot[1] == nil,
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
      hurt_first.animation_selector == E.PlayerHurt.SELECTOR_BASE and
      hurt_last ~= nil and hurt_last.animation == "runner_hurt" and
      hurt_last.frame == E.PlayerHurt.FRAME_COUNT - 1 and
      hurt_finished ~= nil and hurt_finished.animation == "idle" and
      hurt_finished.animation_selector == 0 and
      hurt_recovered ~= nil and hurt_recovered.animation == "idle" and
      hurt_recovered.frame == 0,
      "player C072 completes after its finite ROM list and returns to C052")
check(exact_cue,
      "B07E emits the kind-3 B03E cue on the eighth later service")

print(string.format("player_hurt_tests: %d/%d passed", passed, total))
if passed ~= total then os.exit(1) end
