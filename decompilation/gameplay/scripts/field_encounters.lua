-- ROM 0x1E166 / 0x59986 selection and the ordered dynamic actor lists.
-- C++ supplies a normalized weighted-choice sample and consumes the typed
-- result; event policy and encounter content remain in Lua.
-- Event and choice IDs are semantic; source handler operands stay in the
-- offline evidence table and never enter this runtime state.
local Encounters = {}
local E = require("enums")

local function integer(value, minimum, maximum)
    return type(value) == "number" and value % 1 == 0 and
           value >= minimum and value <= maximum
end

local function content()
    local value = game.modules.world_content_data
    if type(value) ~= "table" then
        return nil
    end
    return value
end

local function semantic_id(value)
    return type(value) == "string" and
           value:match("^[%w_.:-]+$") ~= nil
end

local function selection_fraction(value)
    return type(value) == "number" and value == value and
           value >= 0 and value < 1
end

local function actor_from_row(row)
    local actor = {
        sequence_order = row.sequence_order,
        sequence_id = row.sequence_id,
        placement_rule = row.placement_rule,
        actor_name = row.actor_name,
        profile_id = row.profile_id,
        visual_profile_id = row.visual_profile_id,
        action_kind = row.action_kind,
        action_id = row.action_id,
        actor_facts = row.actor_facts,
        physical_meter = row.physical_meter,
        palette_line = row.palette_line,
        sprite_stem = row.sprite_stem,
        activity_probe_enabled = row.activity_probe_enabled,
        action_delay_modifier = row.action_delay_modifier,
        combatant = {
            actor_facts = {
                interaction = row.actor_facts.interaction,
            },
            cyberware = row.combat_cyberware,
            physical_meter = row.physical_meter,
            mental_meter = row.combat_mental_meter,
            posture_reduction_percent = row.posture_reduction_percent,
            attributes = {
                body = row.combat_body,
                quickness = row.combat_quickness,
                strength = row.combat_strength,
                intelligence = row.combat_intelligence,
                willpower = row.combat_willpower,
            },
            skills = {
                sorcery = row.combat_sorcery,
                firearms = row.combat_firearms,
                pistols = row.combat_pistols,
                smgs = row.combat_smgs,
                shotguns = row.combat_shotguns,
                melee_combat = row.combat_melee_combat,
            },
            equipped_armor_id = row.equipped_armor_id,
            combat_traits = row.combat_traits,
            last_action_successes = 0,
            action_profile_options = {
                primary = row.primary_action_profile_enabled,
                secondary = row.secondary_action_profile_enabled,
            },
            force = row.force,
            spell_trial_cap = row.spell_trial_cap,
            weapon_action_policies = row.weapon_action_policies,
            inventory = row.inventory,
        },
    }
    if row.run_target_group_id ~= nil and
       row.run_target_group_id ~= "none" then
        actor.spawn_x = row.run_target_x
        actor.spawn_y = row.run_target_y
    end
    return actor
end

local function ordered_actor_rows(data, predicate)
    local actors = {}
    for _, row in ipairs(data.dynamic_world_activity_actors or {}) do
        if predicate(row) then
            actors[#actors + 1] = actor_from_row(row)
        end
    end
    table.sort(actors, function(left, right)
        return left.sequence_order < right.sequence_order
    end)
    return actors
end

local function placement_policy_for_rule(placement_rule)
    if placement_rule == "explicit_actor_positions" then
        return "explicit_actor_positions"
    end
    if placement_rule ==
       "open screen-edge candidates based on the current viewport and terrain" then
        return "screen_edge_candidates"
    end
    if placement_rule ==
       "map-derived candidates around camera-relative map anchors" then
        return "map_derived_candidates"
    end
    return nil
end

-- ROM 0x1E166 owns the encounter attempt policy. The host supplies typed
-- world facts and performs the source PRNG draw only when this function asks
-- for one, so blocked/not-due field passes do not consume random state.
function Encounters.trigger_attempt(context)
    if type(context) ~= "table" or
       not integer(context.scene_id, 0, E.Scene.LAST_ID) or
       not integer(context.frame_counter, 0, E.FieldAlarm.WORD_MAX) or
       not integer(context.display_service_word, 0, E.FieldAlarm.WORD_MAX) or
       type(context.leader_protected) ~= "boolean" or
       type(context.field_activity_lock_active) ~= "boolean" or
       type(context.field_alarm_active) ~= "boolean" or
       type(context.combat_active) ~= "boolean" or
       not integer(context.encounter_actor_count, 0,
                   E.FieldAlarm.ACTOR_COUNT_MAX) then
        return nil
    end

    if context.display_service_word ~= 0 or
       context.leader_protected or
       context.field_activity_lock_active or
       (not context.field_alarm_active and context.combat_active) or
       context.encounter_actor_count ~= 0 then
        return "blocked"
    end
    if (context.frame_counter & E.FieldEncounter.FRAME_MASK) ~=
       E.FieldEncounter.FRAME_VALUE then
        return "not_due"
    end
    if context.probability_roll == nil then
        return "sample_probability"
    end
    if not integer(context.probability_roll, 0, 0xffff) or
       not integer(context.probability_threshold, 0, 0xffff) then
        return nil
    end
    if context.probability_roll >= context.probability_threshold then
        return "random_miss"
    end
    if context.field_alarm_active then
        return "select_alarm_sequence"
    end
    return "select_scene_outcome"
end

function Encounters.resolve(scene_id, weighted_selection)
    if not integer(scene_id, 0, E.Scene.LAST_ID) or
       not selection_fraction(weighted_selection) then
        return nil
    end

    local data = content()
    if data == nil then
        return nil
    end
    local selected
    local total_weight = 0
    for _, row in ipairs(data.world_activity_outcomes or {}) do
        if row.scene_id == scene_id then
            if not integer(row.chance_weight, 1, 25) or
               not semantic_id(row.outcome_id) then
                return nil
            end
            total_weight = total_weight + row.chance_weight
        end
    end
    if total_weight ~= 25 then
        return nil
    end
    local ticket = math.floor(weighted_selection * total_weight)
    local cumulative_weight = 0
    for _, row in ipairs(data.world_activity_outcomes or {}) do
        if row.scene_id == scene_id then
            cumulative_weight = cumulative_weight + row.chance_weight
            if ticket < cumulative_weight then
                selected = row
                break
            end
        end
    end
    if selected == nil then return nil end

    if selected.outcome_kind == "dialogue_event" then
        local event_id = selected.outcome_id:match("^event%.([%w_.:-]+)$")
        assert(semantic_id(event_id),
               "world activity outcome has no semantic event ID")
        local opening_text
        for _, event in ipairs(data.world_activity_event_content or {}) do
            if event.event_id == event_id then
                opening_text = event.opening_text
                break
            end
        end
        assert(type(opening_text) == "string" and opening_text ~= "",
               "world activity event has no opening text")
        local semantic_choices = {}
        for _, choice in ipairs(data.world_activity_event_choices or {}) do
            if choice.event_id == event_id then
                assert(choice.choice_order == #semantic_choices + 1 and
                       semantic_id(choice.choice_id) and
                       type(choice.label) == "string" and
                       choice.label ~= "",
                       "world activity outcome has an invalid semantic choice")
                semantic_choices[#semantic_choices + 1] = {
                    id = choice.choice_id,
                    label = choice.label,
                }
            end
        end
        return {
            kind = "dialogue_event",
            scene_id = scene_id,
            outcome_id = selected.outcome_id,
            event_id = event_id,
            opening_text = opening_text,
            choices = semantic_choices,
        }
    end
    if selected.outcome_kind ~= "dynamic_world_effect_sequence" then
        return nil
    end

    local actors = {}
    actors = ordered_actor_rows(data, function(row)
        return row.selection_kind == "weighted_encounter" and
               row.scene_id == scene_id and
               row.sequence_id == selected.outcome_id
    end)
    if #actors == 0 then
        return nil
    end
    local placement_rule = actors[1].placement_rule
    for _, actor in ipairs(actors) do
        if actor.placement_rule ~= placement_rule then
            return nil
        end
    end
    local placement_policy = placement_policy_for_rule(placement_rule)
    if placement_policy == nil then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = placement_policy,
        scene_id = scene_id,
        outcome_id = selected.outcome_id,
        sequence_id = selected.outcome_id,
        actors = actors,
    }
end

function Encounters.resolve_run_target_actor_group(group_id)
    if type(group_id) ~= "string" or
       not group_id:match("^[%w_.:-]+$") then
        return nil
    end
    return Encounters.resolve_sequence(15, group_id)
end

-- A named actor sequence can be started by a story command without consuming
-- the ordinary field random selector. Import resolves source list addresses
-- to these stable IDs before the table reaches Lua.
function Encounters.resolve_sequence(scene_id, sequence_id)
    if not integer(scene_id, 0, E.Scene.LAST_ID) or
       type(sequence_id) ~= "string" or
       not sequence_id:match("^[%w_.:-]+$") then
        return nil
    end
    local data = content()
    if data == nil then
        return nil
    end
    local actors = ordered_actor_rows(data, function(row)
        return row.scene_id == scene_id and
               row.sequence_id == sequence_id and
               (row.selection_kind == "named_sequence" or
                (row.selection_kind == "run_target_group" and
                 row.run_target_group_id == sequence_id))
    end)
    if #actors == 0 then
        return nil
    end
    local placement_rule = actors[1].placement_rule
    for _, actor in ipairs(actors) do
        if actor.placement_rule ~= placement_rule then
            return nil
        end
    end
    local placement_policy = placement_policy_for_rule(placement_rule)
    if placement_policy == nil then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = placement_policy,
        scene_id = scene_id,
        outcome_id = sequence_id,
        sequence_id = sequence_id,
        actors = actors,
    }
end

-- ROM 0x1E1DA selects the fixed alternate list at 0x1E20A while FFE16F
-- (field_alarm_active) is set. The list's actor template pointer is 0x1D6A1C.
function Encounters.resolve_alarm_sequence(scene_id)
    return Encounters.resolve_sequence(
        scene_id, "field_alarm_reinforcements")
end

return Encounters
