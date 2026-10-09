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
    }
end

return Building
