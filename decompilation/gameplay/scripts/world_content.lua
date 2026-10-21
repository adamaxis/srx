-- Generic access to editor-authored map entities. The native Lua package
-- loader reads gameplay/world TSV files into game.modules.world_content_data;
-- this module supplies the reusable lookup behavior used by map scripts.
local World = {}

local function content()
    local data = game.modules.world_content_data
    if type(data) ~= "table" then
        return {
            entities = {}, areas = {}, map_triggers = {}, map_events = {},
            actor_spawns = {}, encounter_regions = {},
            world_activity_outcomes = {},
            dynamic_world_activity_actors = {},
            world_activity_actor_lists = {},
        }
    end
    return data
end

function World.entity(entity_id)
    if type(entity_id) ~= "string" then
        return nil
    end
    return content().entities[entity_id]
end

function World.instances_in_scene(scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    return content().areas[scene_id] or {}
end

local function records_in_scene(field, scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    local records = content()[field]
    if type(records) ~= "table" then
        return {}
    end
    return records[scene_id] or {}
end

function World.triggers_in_scene(scene_id)
    return records_in_scene("map_triggers", scene_id)
end

-- Runtime map-event rules are also authored with the area's other map data.
-- Their exact coordinate scope, trigger type, state gates, and repeat policy
-- are read directly from the viewer TSV records into the runtime data table.
function World.events_in_scene(scene_id)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 then
        return {}
    end
    local events = content().map_events or {}
    local result = {}
    for _, event in ipairs(events) do
        if event.scene == scene_id then
            result[#result + 1] = event
        end
    end
    return result
end

-- A location link binds a ROM collision event to the entity it belongs to.
-- More than one row may describe the same entrance when route conditions
-- select different source-verified outcomes. The Lua route policy resolves
-- those variants; the world index only returns their authored data.
function World.location_links_for_event(scene_id, event_code)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       type(event_code) ~= "string" then
        return {}
    end
    local result = {}
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        for _, link in ipairs(entity and entity.links or {}) do
            if link.source_scene == scene_id and
               link.source_event == event_code then
                result[#result + 1] = link
            end
        end
    end
    return result
end

-- Keep the unique-link convenience API for callers that do not need
-- conditional variants. Ambiguous bindings remain unresolved here.
function World.location_link_for_event(scene_id, event_code)
    local links = World.location_links_for_event(scene_id, event_code)
    if #links == 1 then return links[1] end
    return nil
end

function World.actor_spawns_in_scene(scene_id)
    return records_in_scene("actor_spawns", scene_id)
end

-- Return the ROM-selected conditional population as ordinary host actor
-- descriptors. The selector index is resolved by the field activity policy;
-- this lookup consumes extracted scene/list data and never follows ROM
-- pointers at runtime. Entries with script_index -1 are event-driven lists
-- consumed by their own activity route.
function World.world_activity_actors_for_script(scene_id, script_index)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       type(script_index) ~= "number" or script_index % 1 ~= 0 or
       script_index < 0 then
        return {}
    end
    local rows = content().world_activity_actor_lists or {}
    local result = {}
    for _, row in ipairs(rows) do
        if row.scene == scene_id and row.script_index == script_index then
            result[#result + 1] = {
                id = "world_activity:" .. scene_id .. ":" .. script_index ..
                    ":" .. row.source_order,
                scene = row.scene,
                script_index = row.script_index,
                source_order = row.source_order,
                list_offset = row.list_offset,
                source_record_offset = row.source_record_offset,
                tagged_template = row.tagged_template,
                template_offset = row.template_offset,
                position_offset = row.position_offset,
                name = row.actor_name,
                profile_id = row.profile_id,
                entity_kind = row.entity_kind,
                palette_line = row.palette_line,
                sprite = row.sprite_stem,
                x = row.world_x,
                y = row.world_y,
                heading = row.initial_heading,
                action_kind = row.action_kind,
                action_code = row.action_code,
                source_action_slot = row.source_action_slot,
                source_action_ammo = row.source_action_ammo,
                source_action_counter = row.source_action_counter,
                ordered_overrides = row.ordered_overrides,
                source_spawn_state_flags = row.source_spawn_state_flags,
                source_interaction_flags = row.source_interaction_flags,
                physical_meter = row.physical_meter,
                delay_primary_attribute = row.delay_primary_attribute,
                delay_secondary_attribute = row.delay_secondary_attribute,
                delay_capability_flags = row.delay_capability_flags,
                combatant = {
                    interaction_flags = row.source_interaction_flags,
                    physical_meter = row.physical_meter,
                    mental_meter = row.combat_mental_meter,
                    cyberware_flags = row.combat_cyberware_flags,
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
                    action_profile_modifier_flags =
                        row.combat_modifier_flags_df,
                    equipped_action_code = row.action_code,
                    auxiliary_roll_bonus =
                        row.combat_auxiliary_roll_bonus,
                    item_codes = {
                        row.combat_item_code_0, row.combat_item_code_1,
                        row.combat_item_code_2, row.combat_item_code_3,
                        row.combat_item_code_4, row.combat_item_code_5,
                        row.combat_item_code_6, row.combat_item_code_7,
                    },
                    item_phases = {
                        row.combat_item_phase_0, row.combat_item_phase_1,
                        row.combat_item_phase_2, row.combat_item_phase_3,
                        row.combat_item_phase_4, row.combat_item_phase_5,
                        row.combat_item_phase_6, row.combat_item_phase_7,
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
                    action_skills = {
                        row.combat_action_skill_81,
                        row.combat_action_skill_82,
                        row.combat_action_skill_83,
                        row.combat_action_skill_84,
                    },
                },
            }
        end
    end
    table.sort(result, function(left, right)
        return left.source_order < right.source_order
    end)
    return result
end

function World.encounter_regions_in_scene(scene_id)
    return records_in_scene("encounter_regions", scene_id)
end

-- Collision rectangles belong to reusable entity definitions and are
-- translated by the area's entity origin. Unreviewed records are inspectable
-- in the viewer but are not exposed as runtime collision results.
function World.collision_at(scene_id, x, y)
    if type(scene_id) ~= "number" or type(x) ~= "number" or
       type(y) ~= "number" then
        return nil
    end
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        if entity then
            for _, region in ipairs(entity.collision_regions or {}) do
                local x0 = instance.origin_x + region.local_x
                local y0 = instance.origin_y + region.local_y
                local reviewed = region.review_status == "source_verified" or
                    region.review_status == "new_content"
                if reviewed and region.mode ~= "unreviewed" and
                   x >= x0 and y >= y0 and
                   x < x0 + region.width and
                   y < y0 + region.height then
                    return {
                        instance_id = instance.instance_id,
                        entity_id = entity.id,
                        collision_id = region.id,
                        mode = region.mode,
                        evidence = region.evidence,
                        review_status = region.review_status,
                    }
                end
            end
        end
    end
    return nil
end

-- Resolve a manually authored interaction at map-pixel coordinates. ROM
-- event records remain the authority for existing interactions; authored
-- rectangles provide the reusable extension point for new world content.
function World.interaction_at(scene_id, x, y, source_event)
    if type(scene_id) ~= "number" or type(x) ~= "number" or
       type(y) ~= "number" then
        return nil
    end
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        if entity then
            for _, interaction in ipairs(entity.interactables or {}) do
                local event_matches = source_event == nil or
                    interaction.source_event == "" or
                    interaction.source_event == source_event
                local x0 = instance.origin_x + interaction.local_x
                local y0 = instance.origin_y + interaction.local_y
                if event_matches and x >= x0 and y >= y0 and
                   x < x0 + interaction.width and
                   y < y0 + interaction.height then
                    return {
                        instance_id = instance.instance_id,
                        entity_id = entity.id,
                        lua_handler = interaction.lua_handler,
                        source_event = interaction.source_event,
                        interaction_id = interaction.id,
                    }
                end
            end
        end
    end
    return nil
end

return World
