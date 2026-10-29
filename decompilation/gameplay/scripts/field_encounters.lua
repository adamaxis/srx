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

local function source_hex_word(value)
    if value == nil or value == "" then
        return nil, true
    end
    if type(value) ~= "string" then
        return nil, false
    end
    local digits = value:match("^0[xX]([%da-fA-F]+)$")
    if digits == nil then
        return nil, false
    end
    local parsed = tonumber(digits, 16)
    if not integer(parsed, 0, 0xffffffff) then
        return nil, false
    end
    return parsed, true
end

local function source_dispatch_target(value)
    if value == nil or value == "" then
        return nil, false, true
    end
    if type(value) ~= "string" then
        return nil, false, false
    end
    local returns_from_prompt = value:sub(1, 7) == "return:"
    local address = returns_from_prompt and value:sub(8) or value
    local parsed, valid = source_hex_word(address)
    return parsed, returns_from_prompt, valid
end

local function actor_from_row(row)
    local actor = {
        source_order = row.source_order,
        sequence_id = row.sequence_id,
        placement_rule = row.placement_rule,
        actor_name = row.actor_name,
        profile_id = row.profile_id,
        entity_kind = row.entity_kind,
        action_kind = row.action_kind,
        action_code = row.action_code,
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
            cyberware_flags = row.combat_cyberware_flags,
            physical_meter = row.physical_meter,
            mental_meter = row.combat_mental_meter,
            posture = row.combat_posture,
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
            armor_index = row.combat_armor_index,
            race_or_state = row.combat_race_or_state,
            class_code = row.combat_class_code,
            equipped_action_code = row.action_code,
            action_result = 0,
            action_profile_modifier_flags = row.combat_modifier_flags_df,
            linked_action_profile_index = row.combat_profile_index_a0,
            auxiliary_roll_bonus = row.combat_auxiliary_roll_bonus,
            magic_cap = row.combat_magic_cap,
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
        return left.source_order < right.source_order
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
        local choice_1_command, choice_1_command_valid =
            source_hex_word(selected.choice_1_command)
        local choice_2_command, choice_2_command_valid =
            source_hex_word(selected.choice_2_command)
        local choice_3_command, choice_3_command_valid =
            source_hex_word(selected.choice_3_command)
        local choice_1_target, choice_1_returns, choice_1_target_valid =
            source_dispatch_target(selected.choice_1_dispatch_target)
        local choice_2_target, choice_2_returns, choice_2_target_valid =
            source_dispatch_target(selected.choice_2_dispatch_target)
        local choice_3_target, choice_3_returns, choice_3_target_valid =
            source_dispatch_target(selected.choice_3_dispatch_target)
        assert(choice_1_command_valid and choice_2_command_valid and
               choice_3_command_valid and choice_1_target_valid and
               choice_2_target_valid and choice_3_target_valid,
               "world activity outcome contains an invalid choice route")
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
            choice_1_command = choice_1_command,
            choice_1_dispatch_target = choice_1_target,
            choice_1_dispatch_returns = choice_1_returns,
            choice_2_command = choice_2_command,
            choice_2_dispatch_target = choice_2_target,
            choice_2_dispatch_returns = choice_2_returns,
            choice_3_command = choice_3_command,
            choice_3_dispatch_target = choice_3_target,
            choice_3_dispatch_returns = choice_3_returns,
        }
    end
    if selected.outcome_kind ~= "dynamic_world_effect_sequence" then
        return nil
    end

    local actors = {}
    actors = ordered_actor_rows(data, function(row)
        return row.selection_kind == "weighted_encounter" and
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
    local placement_policy = placement_policy_for_rule(placement_rule)
    if placement_policy == nil then
        return nil
    end
    return {
        kind = "dynamic_world_effect_sequence",
        placement_policy = placement_policy,
        scene = scene_id,
        context_group = selected.context_group,
        random_sum = sum,
        sequence_id = string.format(
            "field_encounter.scene.%d.context.%d.weight.%d",
            scene_id, selected.context_group, sum),
        event_name = selected.event_name,
        opening_text = "",
        choice_1 = "",
        choice_2 = "",
        choice_3 = "",
        choice_1_dispatch_returns = false,
        choice_2_dispatch_returns = false,
        choice_3_dispatch_returns = false,
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
        return row.scene == scene_id and
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
        scene = scene_id,
        context_group = 0,
        random_sum = 0,
        sequence_id = sequence_id,
        event_name = "source_sequence",
        opening_text = "",
        choice_1 = "",
        choice_2 = "",
        choice_3 = "",
        choice_1_dispatch_returns = false,
        choice_2_dispatch_returns = false,
        choice_3_dispatch_returns = false,
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
