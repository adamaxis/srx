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

local function actor_row(source_order)
    return {
        scene = 2,
        context_group = 0,
        random_sum = E.FieldEncounter.SOURCE_SEQUENCE_SENTINEL,
        selection_cell_offset = 0,
        sequence_list_offset = E.FieldEncounter.ALARM_SEQUENCE_LIST_ADDRESS,
        source_order = source_order,
        actor_name = "corp guard",
        profile_id = "corp_guard_k05_item06",
        action_kind = "item",
        action_code = 6,
        source_action_slot = 0,
        source_action_ammo = 0,
        source_action_counter = 0,
        source_spawn_state_flags = E.ActorFlag.SPOTTED |
            E.ActorFlag.REINFORCEMENT | E.ActorFlag.RUNNER |
            E.ActorFlag.HOSTILE,
        source_interaction_flags = 0,
        entity_kind = E.SpriteType.CORP_GUARD,
        palette_line = 2,
        sprite_stem = "corp_guard_k05_item06",
        tagged_template = E.FieldEncounter.ALARM_ACTOR_TEMPLATE_ADDRESS,
        template_offset = E.FieldEncounter.ALARM_ACTOR_TEMPLATE_ADDRESS,
        ordered_overrides = "-",
        placement_rule =
            "map-derived candidates around camera-relative map anchors",
        physical_meter = 10,
        delay_primary_attribute = 3,
        delay_secondary_attribute = 2,
        delay_capability_flags = 0,
        combat_mental_meter = 0,
        combat_cyberware_flags = 0,
        combat_posture = E.Posture.NEUTRAL,
        combat_attribute_77 = 0,
        combat_attribute_78 = 0,
        combat_attribute_79 = 0,
        combat_attribute_7b = 0,
        combat_attribute_7c = 0,
        combat_attribute_85 = 0,
        combat_item18_base = 0,
        combat_armor_index = 0,
        combat_race_or_state = 0,
        combat_class_code = 0,
        combat_modifier_flags_df = 0,
        combat_profile_index_a0 = 0,
        combat_item_code_0 = 0,
        combat_item_code_1 = 0,
        combat_item_code_2 = 0,
        combat_item_code_3 = 0,
        combat_item_code_4 = 0,
        combat_item_code_5 = 0,
        combat_item_code_6 = 0,
        combat_item_code_7 = 0,
        combat_item_phase_0 = 0,
        combat_item_phase_1 = 0,
        combat_item_phase_2 = 0,
        combat_item_phase_3 = 0,
        combat_item_phase_4 = 0,
        combat_item_phase_5 = 0,
        combat_item_phase_6 = 0,
        combat_item_phase_7 = 0,
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
        combat_action_skill_81 = 0,
        combat_action_skill_82 = 0,
        combat_action_skill_83 = 0,
        combat_action_skill_84 = 0,
        combat_auxiliary_roll_bonus = 0,
    }
end

game = {
    modules = {
        world_content_data = {
            dynamic_world_activity_actors = {
                actor_row(1),
                actor_row(0),
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

local alarm_sequence = Encounters.resolve_alarm_sequence(2)
eq(alarm_sequence.kind, "dynamic_world_effect_sequence",
   "alarm list resolves as a dynamic sequence")
eq(alarm_sequence.sequence_list_offset,
   E.FieldEncounter.ALARM_SEQUENCE_LIST_ADDRESS,
   "alarm sequence retains the 0x1E20A table provenance")
eq(alarm_sequence.placement_policy, "map_derived_candidates",
   "alarm sequence uses the ROM 0x1E302 map placement path")
eq(#alarm_sequence.actors, 2, "alarm table resolves its two actor pointers")
eq(alarm_sequence.actors[1].source_order, 0,
   "alarm actors retain source order")
eq(alarm_sequence.actors[1].template_offset,
   E.FieldEncounter.ALARM_ACTOR_TEMPLATE_ADDRESS,
   "alarm actors use source template 0x1D6A1C")
eq(alarm_sequence.actors[1].source_spawn_state_flags,
   E.ActorFlag.SPOTTED | E.ActorFlag.REINFORCEMENT |
       E.ActorFlag.RUNNER | E.ActorFlag.HOSTILE,
   "alarm actor template flags are preserved for the common spawn path")

print(("field encounter tests passed (%d checks)"):format(checks))
