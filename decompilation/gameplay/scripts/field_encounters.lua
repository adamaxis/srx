-- ROM 0x1E166 / 0x59986 selection and the ordered dynamic actor lists.
-- C++ supplies the source PRNG draws and consumes the selected typed result;
-- event policy and encounter content remain in Lua.
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

local function actor_from_row(row)
    local actor = {
        source_order = row.source_order,
        selection_cell_offset = row.selection_cell_offset,
        sequence_list_offset = row.sequence_list_offset,
        placement_rule = row.placement_rule,
        actor_name = row.actor_name,
        profile_id = row.profile_id,
        entity_kind = row.entity_kind,
        action_kind = row.action_kind,
        action_code = row.action_code,
        source_action_slot = row.source_action_slot,
        source_action_ammo = row.source_action_ammo,
        source_action_counter = row.source_action_counter,
        source_interaction_flags = row.source_interaction_flags,
        physical_meter = row.physical_meter,
        palette_line = row.palette_line,
        sprite_stem = row.sprite_stem,
        tagged_template = row.tagged_template,
        template_offset = row.template_offset,
        ordered_overrides = row.ordered_overrides,
        delay_primary_attribute = row.delay_primary_attribute,
        delay_secondary_attribute = row.delay_secondary_attribute,
        delay_capability_flags = row.delay_capability_flags,
        combatant = {
            interaction_flags = row.source_interaction_flags,
            cyberware_flags = row.combat_cyberware_flags,
            physical_meter = row.physical_meter,
            mental_meter = row.combat_mental_meter,
            posture = row.combat_posture,
            command_attribute_77 = row.combat_attribute_77,
            command_attribute_78 = row.combat_attribute_78,
            command_attribute_79 = row.combat_attribute_79,
            command_attribute_7b = row.combat_attribute_7b,
            command_attribute_7c = row.combat_attribute_7c,
            command_attribute_85 = row.combat_attribute_85,
            item18_base = row.combat_item18_base,
            armor_index = row.combat_armor_index,
            race_or_state = row.combat_race_or_state,
            class_code = row.combat_class_code,
            equipped_action_code = row.action_code,
            action_result = 0,
            action_profile_modifier_flags = row.combat_modifier_flags_df,
            linked_action_profile_index = row.combat_profile_index_a0,
            auxiliary_roll_bonus = row.combat_auxiliary_roll_bonus,
            action_skills = {
                row.combat_action_skill_81,
                row.combat_action_skill_82,
                row.combat_action_skill_83,
                row.combat_action_skill_84,
            },
            action_phase_extension = {
                row.combat_action_phase_76,
                row.combat_action_phase_77,
                row.combat_action_phase_78,
                row.combat_action_phase_79,
                row.combat_action_phase_7a,
                row.combat_action_phase_7b,
                row.combat_action_phase_7c,
                row.combat_action_phase_7d,
                row.combat_action_phase_7e,
                row.combat_action_phase_7f,
            },
            item_codes = {
                row.combat_item_code_0,
                row.combat_item_code_1,
                row.combat_item_code_2,
                row.combat_item_code_3,
                row.combat_item_code_4,
                row.combat_item_code_5,
                row.combat_item_code_6,
                row.combat_item_code_7,
            },
            item_phases = {
                row.combat_item_phase_0,
                row.combat_item_phase_1,
                row.combat_item_phase_2,
                row.combat_item_phase_3,
                row.combat_item_phase_4,
                row.combat_item_phase_5,
                row.combat_item_phase_6,
                row.combat_item_phase_7,
            },
        },
    }
    if (row.run_target_area or 0) ~= 0 then
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
        return left.source_order < right.source_order
    end)
    return actors
end

-- ROM 0x1E166 owns the encounter attempt policy. The host supplies typed
-- world facts and performs the source PRNG draw only when this function asks
-- for one, so blocked/not-due field passes do not consume random state.
function Encounters.trigger_attempt(context)
    if type(context) ~= "table" or
       not integer(context.scene_id, 0, E.Scene.LAST_ID) or
       not integer(context.frame_counter, 0, 0xffff) or
       type(context.trigger_delay_active) ~= "boolean" or
       type(context.current_entity_protected) ~= "boolean" or
       type(context.activity_lock) ~= "boolean" or
       type(context.sequence_mode) ~= "boolean" or
       type(context.world_effects_active) ~= "boolean" or
       type(context.placement_pending) ~= "boolean" then
        return nil
    end

    if context.trigger_delay_active or
       context.current_entity_protected or
       context.activity_lock or
       (not context.sequence_mode and context.world_effects_active) or
       context.placement_pending then
        return "blocked"
    end
    if (context.frame_counter & 0x03ff) ~= 10 then
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
    if context.sequence_mode then
        return "forward_active_sequence"
    end
    return "select_scene_outcome"
end

function Encounters.resolve(scene_id, first_roll, second_roll)
    if not integer(scene_id, 0, E.Scene.LAST_ID) or
       not integer(first_roll, 0, 4) or
       not integer(second_roll, 0, 4) then
        return nil
    end

    local data = content()
    if data == nil then
        return nil
    end
    local sum = first_roll + second_roll
    local selected
    for _, row in ipairs(data.world_activity_outcomes or {}) do
        if row.scene == scene_id and row.random_sum == sum then
            selected = row
            break
        end
    end
    if selected == nil then
        return nil
    end

    if selected.outcome_kind == "dialogue_event" then
        return {
            kind = "dialogue_event",
            scene = scene_id,
            context_group = selected.context_group,
            random_sum = sum,
            event_name = selected.event_name,
            handler_offset = selected.handler_offset,
            opening_text = selected.opening_text,
            choice_1 = selected.choice_1,
            choice_2 = selected.choice_2,
            choice_3 = selected.choice_3,
            choice_1_command = selected.choice_1_command,
            choice_1_dispatch_target = selected.choice_1_dispatch_target,
            choice_2_command = selected.choice_2_command,
            choice_2_dispatch_target = selected.choice_2_dispatch_target,
            choice_3_command = selected.choice_3_command,
            choice_3_dispatch_target = selected.choice_3_dispatch_target,
        }
    end
    if selected.outcome_kind ~= "dynamic_world_effect_sequence" then
        return nil
    end

    local actors = {}
    actors = ordered_actor_rows(data, function(row)
        return (row.run_target_area or 0) == 0 and
               row.scene == scene_id and row.random_sum == sum
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
    local placement_policy
    if placement_rule ==
       "open screen-edge candidates based on the current viewport and terrain" then
        placement_policy = "screen_edge_candidates"
    elseif placement_rule ==
       "map-derived candidates around camera-relative map anchors" then
        placement_policy = "map_derived_candidates"
    else
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = placement_policy,
        scene = scene_id,
        context_group = selected.context_group,
        random_sum = sum,
        event_name = selected.event_name,
        opening_text = "",
        choice_1 = "",
        choice_2 = "",
        choice_3 = "",
        actors = actors,
    }
end

function Encounters.resolve_run_target_actor_group(area_id, event_id)
    if not integer(area_id, 0, E.Scene.LAST_ID) or
       not integer(event_id, 0, 255) then
        return nil
    end
    local data = content()
    if data == nil then
        return nil
    end
    local actors = ordered_actor_rows(data, function(row)
        return row.run_target_area == area_id and
               row.run_target_event == event_id
    end)
    if #actors == 0 then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = "source_actor_positions",
        scene = 15,
        context_group = area_id,
        random_sum = event_id,
        event_name = "gang_contract_target",
        opening_text = "",
        choice_1 = "",
        choice_2 = "",
        choice_3 = "",
        actors = actors,
    }
end

-- A named source list can be started by a story command without consuming the
-- ordinary field random selector. Its actor rows use random_sum 0xFF in the
-- extracted table so this lookup never leaks them into resolve().
function Encounters.resolve_source_sequence(scene_id, sequence_list_offset)
    if not integer(scene_id, 0, E.Scene.LAST_ID) or
       not integer(sequence_list_offset, 0, 0x1fffff) then
        return nil
    end
    local data = content()
    if data == nil then
        return nil
    end
    local actors = ordered_actor_rows(data, function(row)
        return row.scene == scene_id and
               row.random_sum == 0xff and
               row.sequence_list_offset == sequence_list_offset and
               row.placement_rule ==
                   "map-derived candidates around camera-relative map anchors"
    end)
    if #actors == 0 then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = "map_derived_candidates",
        scene = scene_id,
        context_group = 0,
        random_sum = 0,
        event_name = "source_sequence",
        opening_text = "",
        choice_1 = "",
        choice_2 = "",
        choice_3 = "",
        sequence_list_offset = sequence_list_offset,
        actors = actors,
    }
end

return Encounters
