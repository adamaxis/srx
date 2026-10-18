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
-- The rows live beside the reusable entity profile, while the router remains
-- generic. Only source-verified links are authored here; unmapped events keep
-- using the ROM event table supplied by the host.
function World.location_link_for_event(scene_id, event_code)
    if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
       type(event_code) ~= "string" then
        return nil
    end
    local result
    for _, instance in ipairs(World.instances_in_scene(scene_id)) do
        local entity = World.entity(instance.entity_id)
        for _, link in ipairs(entity and entity.links or {}) do
            if link.source_scene == scene_id and
               link.source_event == event_code then
                if result ~= nil then
                    -- Duplicate event bindings make the location ambiguous.
                    -- Do not silently pick whichever entity loaded first.
                    return nil
                end
                result = link
            end
        end
    end
    return result
end

function World.actor_spawns_in_scene(scene_id)
    return records_in_scene("actor_spawns", scene_id)
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
