-- ROM 0x1E166 / 0x59986 selection and the ordered dynamic actor lists.
-- C++ supplies the source PRNG draws and consumes the selected typed result;
-- event policy and encounter content remain in Lua.
local Encounters = {}

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

-- ROM 0x1E166 owns the encounter attempt policy. The host supplies typed
-- world facts and performs the source PRNG draw only when this function asks
-- for one, so blocked/not-due field passes do not consume random state.
function Encounters.trigger_attempt(context)
    if type(context) ~= "table" or
       not integer(context.scene_id, 0, 55) or
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
    if not integer(scene_id, 0, 55) or
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
    for _, row in ipairs(data.dynamic_world_activity_actors or {}) do
        if row.scene == scene_id and row.random_sum == sum then
            actors[#actors + 1] = {
                source_order = row.source_order,
                selection_cell_offset = row.selection_cell_offset,
                sequence_list_offset = row.sequence_list_offset,
                actor_name = row.actor_name,
                profile_id = row.profile_id,
                entity_kind = row.entity_kind,
                action_kind = row.action_kind,
                action_code = row.action_code,
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
            }
        end
    end
    table.sort(actors, function(left, right)
        return left.source_order < right.source_order
    end)
    if #actors == 0 then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
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

return Encounters
