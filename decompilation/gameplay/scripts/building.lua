-- Generic field-location routing. Collision geometry and event-table bytes
-- are ROM-derived map data supplied by the native host; this module selects
-- the location interaction or the table destination without naming unknown
-- places or inventing an interior.
local Building = {}

local route_kind = {
    no_route = "no_route",
    dialogue = "dialogue",
    field_service = "field_service",
    area_event = "area_event",
}

local route_script = {
    none = "none",
    stoker_motel = "stoker_motel_interaction",
    ares_emporium = "ares_weapon_emporium",
    map_event_table = "area_event_table",
}

local destination_mode = {
    field = "field",
    resource_activity = "resource_activity",
    scripted_activity = "scripted_activity",
    halted = "halted",
}

local destination_range = {
    last_field_scene = 20,
    last_resource_activity = 43,
    last_scripted_activity = 55,
}

local function classify_area_event_destination(scene_id)
    if scene_id <= destination_range.last_field_scene then
        return destination_mode.field
    elseif scene_id <= destination_range.last_resource_activity then
        return destination_mode.resource_activity
    elseif scene_id <= destination_range.last_scripted_activity then
        return destination_mode.scripted_activity
    end
    return destination_mode.halted
end

local function no_route(event_code)
    return {
        kind = route_kind.no_route,
        script = route_script.none,
        event_code = event_code,
    }
end

function Building.resolve_field_location_route(context)
    if type(context) ~= "table" or
       type(context.stoker_motel_street) ~= "boolean" or
       type(context.building_interaction_mode) ~= "boolean" or
       type(context.event_code) ~= "string" or
       type(context.event_index) ~= "number" or
       context.event_index < 0 or context.event_index > 255 then
        return no_route("event_0")
    end

    -- ROM scene-2 event 2 is Stoker's same-map motel interaction. It does not
    -- load a Stoker interior; the opening story and later motel states use the
    -- generic dialogue/menu services. Event 11 selects the Ares service.
    if context.stoker_motel_street and
       context.building_interaction_mode then
        if context.event_code == "event_2" then
            return {
                kind = route_kind.dialogue,
                script = route_script.stoker_motel,
                event_code = context.event_code,
            }
        end
        if context.event_code == "event_11" then
            return {
                kind = route_kind.field_service,
                script = route_script.ares_emporium,
                event_code = context.event_code,
            }
        end
    end

    if context.event_table_available ~= true or
       type(context.event_table) ~= "table" or
       context.event_index >= #context.event_table then
        return no_route(context.event_code)
    end

    local target_scene = context.event_table[context.event_index + 1]
    if type(target_scene) ~= "number" or target_scene < 0 or
       target_scene > 255 or target_scene % 1 ~= 0 then
        return no_route(context.event_code)
    end

    return {
        kind = route_kind.area_event,
        script = route_script.map_event_table,
        event_code = context.event_code,
        area_event_target = target_scene,
        area_event_destination_mode =
            classify_area_event_destination(target_scene),
    }
end

-- Elevator trigger scenes and their child scene lists are supplied from the
-- extracted US ROM records. The script owns the entry action, entry floor,
-- and selected destination; the native menu owns only selection and timing.
function Building.resolve_field_location_entry(context)
    if type(context) ~= "table" or
       type(context.scene_id) ~= "number" or
       context.scene_id % 1 ~= 0 or context.scene_id < 0 or
       type(context.event_code) ~= "string" or
       type(context.source_elevator_groups) ~= "table" then
        return nil
    end

    if context.event_code ~= "event_29" then
        return { action = "no_action" }
    end

    local group = context.source_elevator_groups[context.scene_id + 1]
    if type(group) ~= "table" or #group == 0 then
        return { action = "no_action" }
    end
    if #group < 2 or #group > 5 then
        return nil
    end

    local entry_floor = 0
    local seen = {}
    for floor, scene_id in ipairs(group) do
        if type(scene_id) ~= "number" or scene_id % 1 ~= 0 or
           scene_id < 0 or scene_id > 255 or seen[scene_id] then
            return nil
        end
        seen[scene_id] = true
        if scene_id == context.scene_id then
            entry_floor = floor - 1
        end
    end

    return {
        action = "open_elevator_selector",
        floor_scenes = group,
        entry_floor = entry_floor,
    }
end

function Building.resolve_elevator_destination(context)
    if type(context) ~= "table" or
       type(context.floor_scenes) ~= "table" or
       type(context.selected_floor) ~= "number" or
       context.selected_floor % 1 ~= 0 or
       context.selected_floor < 0 or
       context.selected_floor >= #context.floor_scenes then
        return nil
    end

    local destination = context.floor_scenes[context.selected_floor + 1]
    if type(destination) ~= "number" or destination % 1 ~= 0 or
       destination < 0 or destination > 255 then
        return nil
    end
    return destination
end

return Building
