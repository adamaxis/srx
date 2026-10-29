package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local E = require("enums")
local Encounters = require("field_encounters")

local checks = 0
local function eq(actual, expected, label)
    assert(actual == expected,
        ("%s: expected %s, got %s"):format(label, tostring(expected),
                                             tostring(actual)))
    checks = checks + 1
end

local function context(overrides)
    local value = {
        scene_id = 2,
        frame_counter = E.FieldEncounter.FRAME_VALUE,
        display_service_word = 0,
        leader_protected = false,
        field_activity_lock_active = false,
        field_alarm_active = false,
        combat_active = false,
        encounter_actor_count = 0,
        probability_threshold = 5,
    }
    for key, item in pairs(overrides or {}) do
        value[key] = item
    end
    return value
end

local function actor_row(source_order, sequence_id, placement_rule)
    return {
        scene = 2,
        context_group = 0,
        random_sum = E.FieldEncounter.SOURCE_SEQUENCE_SENTINEL,
        sequence_id = sequence_id or "field_alarm_reinforcements",
        source_order = source_order,
        actor_name = "corp guard",
        profile_id = "corp_guard_k05_item06",
        action_kind = "item",
        action_code = 6,
        source_action_slot = 0,
        source_action_ammo = 0,
        source_action_counter = 0,
        actor_facts = {
            roles = {
                spotted = true,
                reinforcement = true,
                runner = true,
                hostile = true,
            },
            interaction = {},
        },
        entity_kind = E.SpriteType.CORP_GUARD,
        palette_line = 2,
        sprite_stem = "corp_guard_k05_item06",
        activity_probe_enabled = false,
        ordered_overrides = "-",
        placement_rule = placement_rule or
            "open screen-edge candidates based on the current viewport and terrain",
        physical_meter = 10,
        delay_primary_attribute = 3,
        delay_secondary_attribute = 2,
        delay_capability_flags = 0,
        combat_mental_meter = 0,
        combat_cyberware_flags = 0,
        combat_posture = E.Posture.NEUTRAL,
        combat_body = 0,
        combat_quickness = 0,
        combat_strength = 0,
        combat_intelligence = 0,
        combat_willpower = 0,
        combat_melee_combat = 0,
        combat_sorcery = 0,
        combat_armor_index = 0,
        combat_race_or_state = 0,
        combat_class_code = 0,
        combat_modifier_flags_df = 0,
        combat_profile_index_a0 = 0,
        inventory = {},
        combat_action_phase_76 = 0,
        combat_action_phase_77 = 0,
        combat_action_phase_78 = 0,
        combat_action_phase_79 = 0,
        combat_action_phase_7a = 0,
        combat_action_phase_7b = 0,
        combat_action_phase_7c = 0,
        combat_action_phase_7d = 0,
        combat_action_phase_7e = 0,
        combat_action_phase_7f = 0,
        combat_firearms = 0,
        combat_pistols = 0,
        combat_smgs = 0,
        combat_shotguns = 0,
        combat_auxiliary_roll_bonus = 0,
    }
end

game = {
    modules = {
        world_content_data = {
            world_activity_outcomes = {
                {
                    scene = 0,
                    context_group = 0,
                    random_sum = 6,
                    outcome_kind = "dialogue_event",
                    event_name = "grenade seller",
                    handler_offset = 0x2112C,
                    opening_text = "A man calls from an alley.",
                    choice_1 = "<Attack!!>",
                    choice_2 = "Buy the grenades.",
                    choice_3 = "<Walk away.>",
                    choice_1_command = "0xF1",
                    choice_1_dispatch_target = "0x24C4C",
                    choice_2_command = "0xF2",
                    choice_2_dispatch_target = "0x24C96",
                    choice_3_command = "0x121670",
                    choice_3_dispatch_target = "return:0x121670",
                },
            },
            dynamic_world_activity_actors = {
                actor_row(1),
                actor_row(0),
                actor_row(1, "boris_strike_team",
                    "map-derived candidates around camera-relative map anchors"),
                actor_row(0, "boris_strike_team",
                    "map-derived candidates around camera-relative map anchors"),
            },
        },
    },
}

eq(Encounters.trigger_attempt(context()), "sample_probability",
   "eligible cadence requests a probability draw")
eq(Encounters.trigger_attempt(context({
       frame_counter = E.FieldEncounter.FRAME_VALUE - 1,
   })), "not_due", "cadence mask waits until the source phase")
eq(Encounters.trigger_attempt(context({ display_service_word = 1 })),
   "blocked", "nonzero FFDFE8 blocks the attempt")
eq(Encounters.trigger_attempt(context({ leader_protected = true })),
   "blocked", "leader +0x11 bit 7 blocks the attempt")
eq(Encounters.trigger_attempt(context({ field_activity_lock_active = true })),
   "blocked", "FFE18B blocks the attempt")
eq(Encounters.trigger_attempt(context({ combat_active = true })),
   "blocked", "FFDF84 blocks while the alarm latch is clear")
eq(Encounters.trigger_attempt(context({
       combat_active = true,
       field_alarm_active = true,
   })), "sample_probability",
   "active alarm latch permits the attempt through the FFDF84 gate")
eq(Encounters.trigger_attempt(context({ encounter_actor_count = 1 })),
   "blocked", "nonzero FFE177 actor count blocks the attempt")
eq(Encounters.trigger_attempt(context({
       probability_roll = 4,
       probability_threshold = 5,
   })), "select_scene_outcome", "below-threshold roll selects a scene outcome")
eq(Encounters.trigger_attempt(context({
       probability_roll = 5,
       probability_threshold = 5,
   })), "random_miss", "equal-threshold roll is rejected")
eq(Encounters.trigger_attempt(context({
       field_alarm_active = true,
       probability_roll = 4,
       probability_threshold = 5,
   })), "select_alarm_sequence",
   "active alarm selects the fixed alternate list after a successful roll")

local grenade_seller = Encounters.resolve(0, 2, 4)
eq(grenade_seller.kind, "dialogue_event",
   "Redmond sum-6 row resolves as a dialogue event")
eq(grenade_seller.choice_1_command, 0xF1,
   "hex command IDs become typed numeric commands")
eq(grenade_seller.choice_1_dispatch_target, 0x24C4C,
   "hex dispatcher addresses become typed targets")
eq(grenade_seller.choice_2_command, 0xF2,
   "second choice command is retained")
eq(grenade_seller.choice_3_command, 0x121670,
   "long prompt return value retains its full source width")
eq(grenade_seller.choice_3_dispatch_target, 0x121670,
   "prompt return value is retained as its typed value")
eq(grenade_seller.choice_3_dispatch_returns, true,
   "return-tagged choice target retains prompt-return semantics")
eq(grenade_seller.choice_1_dispatch_returns, false,
   "ordinary dispatcher target remains a handler call")

local alarm_sequence = Encounters.resolve_alarm_sequence(2)
eq(alarm_sequence.kind, "dynamic_world_effect_sequence",
   "alarm list resolves as a dynamic sequence")
eq(alarm_sequence.sequence_id, "field_alarm_reinforcements",
   "alarm sequence resolves by semantic identity")
eq(alarm_sequence.placement_policy, "screen_edge_candidates",
   "alarm sequence uses the ROM 0x1E302 screen-edge placement path")
eq(alarm_sequence.actors[1].placement_rule,
   "open screen-edge candidates based on the current viewport and terrain",
   "alarm rows retain the source-derived placement metadata")
eq(#alarm_sequence.actors, 2, "alarm table resolves its two actor pointers")
eq(alarm_sequence.actors[1].source_order, 0,
   "alarm actors retain source order")
eq(alarm_sequence.actors[1].sequence_id, "field_alarm_reinforcements",
   "alarm actor belongs to the named reinforcement sequence")
local alarm_actor_facts = alarm_sequence.actors[1].actor_facts
eq(alarm_actor_facts.roles.spotted, true,
   "alarm actor template is marked spotted for the common spawn path")
eq(alarm_actor_facts.roles.reinforcement, true,
   "alarm actor template is marked as reinforcement")
eq(alarm_actor_facts.roles.runner, true,
   "alarm actor template keeps its runner role")
eq(alarm_actor_facts.roles.hostile, true,
   "alarm actor template keeps its hostile role")

local map_sequence = Encounters.resolve_sequence(2, "boris_strike_team")
eq(map_sequence.placement_policy, "map_derived_candidates",
   "named map-derived source lists retain their placement path")
eq(map_sequence.sequence_id, "boris_strike_team",
   "Boris sequence resolves by semantic identity")
eq(#map_sequence.actors, 2,
   "named map-derived source list retains both actor rows")

print(("field encounter tests passed (%d checks)"):format(checks))
