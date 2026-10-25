-- ROM [C 0xAE7E/0xAEB4] enters 0x1033E with +0xA2 == 0. A1 survives from
-- caller/previous-record state into 0x55740; this suite checks the documented
-- native reading of that undefined defender profile and the 0x15830 split.
local E = require("enums")
game = {
    actors = {},
    modules = {
        combat = require("combat"),
        magic = require("magic"),
        items = require("items"),
    },
}
require("actor.init")
local R = require("actor.registry")

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then pass = pass + 1 else print("FAIL " .. label) end
end

local function caster()
    local item_codes, item_phases = {}, {}
    for slot = 1, 8 do
        item_codes[slot] = 0
        item_phases[slot] = 0
    end
    return {
        record_type = 0,
        source_flags = 1,
        physical_meter = 10,
        mental_meter = 0,
        force = 4,
        posture = 2,
        item18_base = 12,
        action_phase_extension = { [10] = 20 },
        action_profile_modifier_flags = 0,
        command_attribute_7c = 12,
        command_attribute_77 = 12,
        race_or_state = 4,
        class_code = 0,
        interaction_flags = 0x80,
        cyberware_flags = 0,
        item_codes = item_codes,
        item_phases = item_phases,
    }
end

local function make_context(action_code)
    local alternate = false
    local occupancy = {}
    for slot = 1, 18 do occupancy[slot] = false end
    return {
        actor_id = "field_player_runner",
        phase = "behavior",
        player_spell_available = true,
        player_spell_code = action_code,
        combat_confirm_held = true,
        combat_action_ready = true,
        combat_input_blocked = false,
        world_scene_state = 0,
        source_record_slot = 0,
        ai_target_candidates = {},
        world_effect_slot_occupancy = occupancy,
        combatant = caster(),
        -- Sixes explode once, then a one makes the accumulated total meet
        -- every tested spell difficulty without consuming an unbounded stream.
        ai_random_bound = function(bound)
            if bound ~= 6 then return nil end
            alternate = not alternate
            return alternate and 5 or 0
        end,
        action_line_clear = function() error("untargeted cast probed a line") end,
    }
end

for action_code = E.SpellActionCode.FLAME_DART,
                    E.SpellActionCode.BARRIER do
    local state = { actor_id = "field_player_runner", action_delay = -1 }
    local context = make_context(action_code)
    local audio = R.begin_player_spell_cast(state, context)
    local result = state.spell_action_resolution
    local is_field_barrier =
        action_code == E.SpellActionCode.SUPER_BARRIER or
        action_code == E.SpellActionCode.BARRIER
    check(state.source_action_dispatch == "player_spell" and
          type(result) == "table" and result.untargeted and
          result.target_id == "" and result.source_record_slot == 0,
          string.format("action %d enters 0x1033E without fabricating a target",
                        action_code))
    check(result ~= nil and result.roll_attempted and
          result.action_result > 0 and result.effect_requested and
          result.difficulty == 6 and
          result.behavior_meter_delta == 1 and
          result.combatant.mental_meter == 1,
          string.format("action %d uses the documented stale-A1 reading, rolls, and drains", action_code))
    if is_field_barrier then
        check(result.field_effect_kind == E.SpellEffectKind.FIELD_BARRIER and
              result.effect_started and result.spell_effect_slot == 0 and
              audio == E.SoundId.FIELD_3B and
              state.player_field_spell_effect ~= nil and
              state.player_field_spell_effect.remaining ==
                  result.action_result *
                      E.SpellTuning.FIELD_BARRIER_TICKS_PER_SUCCESS,
              string.format("action %d spawns 0x49A2 field barrier/audio",
                            action_code))
    else
        check(result.field_effect_kind == E.SpellEffectKind.NONE and
              not result.effect_started and result.spell_effect_slot == nil and
              audio == nil and state.player_field_spell_effect == nil,
              string.format("action %d does not invent a target effect",
                            action_code))
    end
    check(not result.special_result_published,
          string.format("action %d returns at 0x1584C before target/group effects",
                        action_code))
end

local expiring = {
    actor_id = "field_player_runner",
    player_field_spell_effect = {
        action_code = E.SpellActionCode.BARRIER,
        effect_slot = 4,
        remaining = 1,
    },
}
R.advance_spell_effect(expiring, {phase = "render"})
check(expiring.player_field_spell_effect == nil and
      expiring.spell_effect_completed and
      expiring.spell_effect_action_code == E.SpellActionCode.BARRIER and
      expiring.spell_effect_phase == 1 and
      expiring.spell_effect_slot_released == 4,
      "0x4ADC field effect terminal pass releases its shared slot")

print(string.format("player_spell_no_target: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
