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
        encounter_check_due = true,
        display_busy = false,
        leader_unseen = false,
        world_activity_locked = false,
        alarm_active = false,
        combat_active = false,
        encounter_in_progress = false,
        encounter_probability = 5 / 65536,
    }
    for key, item in pairs(overrides or {}) do
        value[key] = item
    end
    return value
end

local function actor_row(sequence_order, sequence_id, placement_rule,
                         scene_id, selection_kind)
    return {
        scene_id = scene_id or 2,
        sequence_order = sequence_order,
        selection_kind = selection_kind or "named_sequence",
        sequence_id = sequence_id or "field_alarm_reinforcements",
        actor_name = "corp guard",
        profile_id = "corp_guard_k05_item06",
        action_kind = "item",
        action_id = "max_power",
        actor_facts = {
            roles = {
                spotted = true,
                reinforcement = true,
                runner = true,
                hostile = true,
            },
            interaction = {},
        },
        visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD,
        palette_line = 2,
        sprite_stem = "corp_guard_k05_item06",
        activity_probe_enabled = false,
        placement_rule = placement_rule or
            "open screen-edge candidates based on the current viewport and terrain",
        physical_meter = 10,
        action_delay_modifier = 0,
        combat_mental_meter = 0,
        combat_cyberware = {
            cybereyes = false,
            hand_razors = false,
            spurs = false,
            smartlink = false,
            melee_strength_bonus = 0,
            melee_defense_bonus = 0,
        },
        posture_reduction_percent = E.Posture.NEUTRAL * 25,
        combat_body = 0,
        combat_quickness = 0,
        combat_strength = 0,
        combat_intelligence = 0,
        combat_willpower = 0,
        combat_melee_combat = 0,
        combat_sorcery = 0,
        equipped_armor_id = "heavy_combat_armor",
        combat_traits = {
            ignores_cover_condition_penalty = false,
            adds_two_condition_difficulty = false,
            is_special_spell_target = false,
            takes_fixed_meter_damage = false,
            qualifies_for_party_support_bonus = true,
            is_ghoul_bounty_target = false,
        },
        primary_action_profile_enabled = false,
        secondary_action_profile_enabled = false,
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
        force = 0,
        spell_trial_cap = 0,
    }
end

game = {
    modules = {
        world_content_data = {
            world_activity_outcomes = {
                {
                    scene_id = 0,
                    chance_weight = 3,
                    outcome_kind = "dialogue_event",
                    outcome_id = "event.alley_grenade_seller",
                },
                {
                    scene_id = 0,
                    chance_weight = 22,
                    outcome_kind = "dynamic_world_effect_sequence",
                    outcome_id = "scene0.world_effect.test_creature",
                    sequence_id = "scene0.world_effect.test_creature",
                },
                {
                    scene_id = 1,
                    chance_weight = 25,
                    outcome_kind = "dialogue_event",
                    outcome_id = "event.variable_choice_probe",
                },
            },
            world_activity_event_content = {
                {
                    event_id = "alley_grenade_seller",
                    opening_text = "A man calls from an alley.",
                },
                {
                    event_id = "variable_choice_probe",
                    opening_text = "Choice-list interface fixture.",
                },
            },
            world_activity_event_choices = {
                {
                    event_id = "alley_grenade_seller",
                    choice_order = 1,
                    choice_id = "attack",
                    label = "<Attack!!>",
                },
                {
                    event_id = "alley_grenade_seller",
                    choice_order = 2,
                    choice_id = "buy_grenades",
                    label = "I'll take the Grenades.",
                },
                {
                    event_id = "alley_grenade_seller",
                    choice_order = 3,
                    choice_id = "leave",
                    label = "<Walk away.>",
                },
                {
                    event_id = "variable_choice_probe",
                    choice_order = 1,
                    choice_id = "choice_one",
                    label = "First option",
                },
                {
                    event_id = "variable_choice_probe",
                    choice_order = 2,
                    choice_id = "choice_two",
                    label = "Second option",
                },
                {
                    event_id = "variable_choice_probe",
                    choice_order = 3,
                    choice_id = "choice_three",
                    label = "Third option",
                },
                {
                    event_id = "variable_choice_probe",
                    choice_order = 4,
                    choice_id = "choice_four",
                    label = "Fourth option",
                },
            },
            dynamic_world_activity_actors = {
                actor_row(1),
                actor_row(0),
                actor_row(1, "boris_strike_team",
                    "map-derived candidates around camera-relative map anchors"),
                actor_row(0, "boris_strike_team",
                    "map-derived candidates around camera-relative map anchors"),
                actor_row(0, "scene0.world_effect.test_creature",
                    "open screen-edge candidates based on the current viewport and terrain",
                    0, "weighted_encounter"),
            },
        },
    },
}

for order, profile_id in ipairs(E.StokerMotelArrival.AMBUSH_PROFILE_IDS) do
    local actor = actor_row(
        order - 1, E.WorldEffectSequence.STOKER_MOTEL_AMBUSH,
        "open screen-edge candidates based on the current viewport and terrain",
        E.Scene.STOKER_MOTEL, "weighted_encounter")
    actor.actor_name = order == 1 and "weenr2" or "weenr1"
    actor.profile_id = profile_id
    actor.visual_profile_id = E.ActorVisualProfile.YAKUZA_WEENR
    actor.action_kind = order == 1 and "item" or "none"
    actor.action_id = order == 1 and "streetline_special" or nil
    actor.sprite_stem = "yakuza_weenr_k08_item0a"
    local actors = game.modules.world_content_data.dynamic_world_activity_actors
    actors[#actors + 1] = actor
end

eq(Encounters.trigger_attempt(context()), "sample_probability",
   "eligible cadence requests a probability draw")
eq(Encounters.trigger_attempt(context({
       encounter_check_due = false,
   })), "not_due", "field clock has no encounter opportunity this pass")
eq(Encounters.trigger_attempt(context({ display_busy = true })),
   "blocked", "busy field presentation blocks the attempt")
eq(Encounters.trigger_attempt(context({ leader_unseen = true })),
   "blocked", "an unseen leader blocks the attempt")
eq(Encounters.trigger_attempt(context({ world_activity_locked = true })),
   "blocked", "locked world activity blocks the attempt")
eq(Encounters.trigger_attempt(context({ combat_active = true })),
   "blocked", "combat blocks while the alarm is clear")
eq(Encounters.trigger_attempt(context({
       combat_active = true,
       alarm_active = true,
   })), "sample_probability",
   "active alarm permits the attempt during combat")
eq(Encounters.trigger_attempt(context({ encounter_in_progress = true })),
   "blocked", "an existing encounter blocks another attempt")
eq(Encounters.trigger_attempt(context({
       probability_sample = 4 / 65536,
   })), "select_scene_outcome", "below-threshold roll selects a scene outcome")
eq(Encounters.trigger_attempt(context({
       probability_sample = 5 / 65536,
   })), "random_miss", "sample equal to the chance boundary is rejected")
eq(Encounters.trigger_attempt(context({
       alarm_active = true,
       probability_sample = 4 / 65536,
   })), "select_alarm_sequence",
   "active alarm selects the fixed alternate list after a successful roll")
eq(Encounters.trigger_attempt(context({
       probability_sample = 65535 / 65536,
       encounter_probability = 65535 / 65536,
   })), "random_miss", "upper source sample equal to the chance boundary misses")

local grenade_seller = Encounters.resolve(0, 0.10)
eq(grenade_seller.kind, "dialogue_event",
   "weighted dialogue outcome resolves by its semantic content")
eq(grenade_seller.outcome_id, "event.alley_grenade_seller",
   "selected outcome exposes its stable content identity")
eq(grenade_seller.event_id, "alley_grenade_seller",
   "selected event has a stable gameplay identity")
eq(grenade_seller.choices[1].id, "attack",
   "first choice carries its named player intent")
eq(grenade_seller.choices[1].label, "<Attack!!>",
   "first choice retains its displayed label")
eq(grenade_seller.choices[2].id, "buy_grenades",
   "second choice carries its named player intent")
eq(grenade_seller.choices[3].id, "leave",
   "third choice carries its named player intent")
eq(grenade_seller.handler_offset, nil,
   "dialogue event state excludes its source handler address")
eq(grenade_seller.choice_1_command, nil,
   "dialogue event state excludes source command words")
eq(grenade_seller.choice_1_dispatch_target, nil,
   "dialogue event state excludes source dispatcher addresses")

local weighted_actor_sequence = Encounters.resolve(0, 0.50)
eq(weighted_actor_sequence.kind, "dynamic_world_effect_sequence",
   "weighted actor group resolves from a semantic outcome")
eq(weighted_actor_sequence.sequence_id,
   "scene0.world_effect.test_creature",
   "weighted actor group uses its content identity")
eq(weighted_actor_sequence.scene_id, 0,
   "encounter result keeps the logical scene identity")
eq(weighted_actor_sequence.context_group, nil,
   "encounter result hides the source context-table index")
eq(weighted_actor_sequence.random_sum, nil,
   "encounter result hides the source selector sum")
eq(#weighted_actor_sequence.actors, 1,
   "semantic group resolves its typed actor list")
local selected_outcome_counts = {}
for ticket = 0, 24 do
    local selected = Encounters.resolve(0, (ticket + 0.5) / 25)
    selected_outcome_counts[selected.outcome_id] =
        (selected_outcome_counts[selected.outcome_id] or 0) + 1
end
eq(selected_outcome_counts["event.alley_grenade_seller"], 3,
   "dialogue entry receives exactly its configured weight")
eq(selected_outcome_counts["scene0.world_effect.test_creature"], 22,
   "actor sequence receives exactly its configured weight")
local variable_choice_event = Encounters.resolve(1, 0.50)
eq(#variable_choice_event.choices, 4,
   "dialogue choices are not fixed to three entries")
eq(variable_choice_event.choices[4].id, "choice_four",
   "later choice entries keep their semantic IDs")

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
eq(alarm_sequence.actors[1].sequence_order, 0,
   "alarm actors retain their list order")
eq(alarm_sequence.actors[1].sequence_id, "field_alarm_reinforcements",
   "alarm actor belongs to the named reinforcement sequence")
eq(alarm_sequence.actors[1].combatant.equipped_armor_id,
   "heavy_combat_armor",
   "encounter actor keeps armor as an item identity")
eq(alarm_sequence.actors[1].combatant.linked_action_profile_index, nil,
   "encounter actor does not carry the ROM action-profile row number")
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

local stoker_ambush = Encounters.resolve_sequence(
    E.Scene.STOKER_MOTEL, E.WorldEffectSequence.STOKER_MOTEL_AMBUSH)
eq(stoker_ambush.kind, "dynamic_world_effect_sequence",
   "Stoker's direct ROM call resolves the shared actor list")
eq(stoker_ambush.sequence_id,
   E.WorldEffectSequence.STOKER_MOTEL_AMBUSH,
   "Stoker ambush uses its semantic source-list identity")
eq(stoker_ambush.placement_policy,
   E.WorldEffectPlacementPolicy.MAP_DERIVED_CANDIDATES,
   "Stoker's direct 0x249C4 call passes D0=1 into the 0x1E40C map-derived path")
eq(#stoker_ambush.actors, E.StokerMotelArrival.AMBUSH_ACTOR_COUNT,
   "Stoker list preserves all three ordered actor records")
for index, profile_id in ipairs(E.StokerMotelArrival.AMBUSH_PROFILE_IDS) do
    eq(stoker_ambush.actors[index].sequence_order, index - 1,
       "Stoker actor keeps its ROM list order " .. index)
    eq(stoker_ambush.actors[index].profile_id, profile_id,
       "Stoker actor keeps its ROM template profile " .. index)
    eq(stoker_ambush.actors[index].action_kind,
       index == 1 and "item" or "none",
       "Stoker actor keeps its ROM action template " .. index)
end

-- Isolate the generic weighted resolver with a full-weight fixture row; the
-- fixture tests caller-specific placement and makes no claim about ROM odds.
game.modules.world_content_data.world_activity_outcomes[#
    game.modules.world_content_data.world_activity_outcomes + 1] = {
        scene_id = E.Scene.STOKER_MOTEL,
        chance_weight = 25,
        outcome_kind = "dynamic_world_effect_sequence",
        outcome_id = E.WorldEffectSequence.STOKER_MOTEL_AMBUSH,
    }
local weighted_stoker_ambush = Encounters.resolve(E.Scene.STOKER_MOTEL, 8 / 25)
eq(weighted_stoker_ambush.kind, "dynamic_world_effect_sequence",
   "weighted Scene-2 selector still resolves the shared Stoker list")
eq(weighted_stoker_ambush.placement_policy,
   E.WorldEffectPlacementPolicy.SCREEN_EDGE_CANDIDATES,
   "ordinary weighted encounter uses the D0=0 0x1E302 screen-edge path")

print(("field encounter tests passed (%d checks)"):format(checks))
