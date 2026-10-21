-- Generic field-location routing. Collision geometry and event-table bytes
-- are ROM-derived map data supplied by the native host; this module selects
-- the location interaction or the table destination without naming unknown
-- places or inventing an interior.
local Building = {}
local World = require("world_content")

local route_kind = {
    no_route = "no_route",
    dialogue = "dialogue",
    field_service = "field_service",
    area_event = "area_event",
    area_event_override = "area_event_override",
}

local route_script = {
    none = "none",
    map_event_table = "area_event_table",
    lua_dialogue = "lua_dialogue",
    host_field_service = "host_field_service",
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

local function return_fields(link)
    local anchor = link.return_anchor
    local facing = link.return_facing
    return {
        return_anchor = anchor ~= "" and anchor or nil,
        return_facing = facing ~= "" and facing or nil,
    }
end

-- Link conditions are small declarative conjunctions over authoritative Lua
-- state facts, for example "story_flag_bit_30=true;story_flag_bit_21=false",
-- or a run state such as "active_run=package_recovery". Separate rows with
-- different priorities represent source-backed branches.
local function condition_matches(clause, context)
    local active_run = clause:match("^active_run=([a-z0-9_]+)$")
    if active_run ~= nil then
        local registry = game.runs and game.runs.registry
        assert(type(registry) == "table" and
               type(registry.active_run_id) == "function",
            "location link active-run condition needs run_registry")
        return registry.active_run_id() == active_run
    end
    local current_run = clause:match("^current_run=([a-z0-9_]+)$")
    if current_run ~= nil then
        local registry = game.runs and game.runs.registry
        assert(type(registry) == "table" and
               type(registry.current_offer_id) == "function",
            "location link current-run condition needs run_registry")
        return registry.current_offer_id() == current_run
    end

    local alternate_result = clause:match("^alternate_result=(-?%d+)$")
    if alternate_result ~= nil then
        return type(context) == "table" and
            context.alternate_result == tonumber(alternate_result)
    end
    local alternate_result_sign = clause:match("^alternate_result=(%a+)$")
    if alternate_result_sign == "positive" or
       alternate_result_sign == "negative" then
        if type(context) ~= "table" or
           type(context.alternate_result) ~= "number" then
            return false
        end
        if alternate_result_sign == "positive" then
            return context.alternate_result > 0
        end
        return context.alternate_result < 0
    end

    local fact, expected = clause:match("^([a-z][a-z0-9_]*)=(%a+)$")
    assert(fact ~= nil and (expected == "true" or expected == "false"),
        "location link condition must name a boolean fact or run id")
    return type(game.state) == "table" and
        game.state[fact] == (expected == "true")
end

local function conditions_match(encoded, context)
    if encoded == nil or encoded == "" or encoded == "-" then
        return true
    end
    assert(type(encoded) == "string" and
           encoded:sub(1, 1) ~= ";" and
           encoded:sub(-1) ~= ";" and
           not encoded:find(";;", 1, true),
        "location link has malformed conditions")
    for clause in encoded:gmatch("[^;]+") do
        if not condition_matches(clause, context) then
            return false
        end
    end
    return true
end

local function link_matches_context(link, context)
    local requires_mode = link.requires_mode
    if requires_mode ~= nil and requires_mode ~= "" and
       not (requires_mode == "building_interaction" and
            context.building_interaction_mode) then
        return false
    end
    return conditions_match(link.route_conditions, context)
end

local function selected_link(links, context)
    local selected
    local selected_priority
    local ambiguous = false
    for _, link in ipairs(links) do
        if link_matches_context(link, context) then
            local priority = link.route_priority or 0
            assert(type(priority) == "number" and priority % 1 == 0,
                "location link priority must be an integer")
            if selected == nil or priority > selected_priority then
                selected = link
                selected_priority = priority
                ambiguous = false
            elseif priority == selected_priority then
                ambiguous = true
            end
        end
    end
    if ambiguous then return nil end
    return selected
end

local function entry_dialogue_fields(link, context)
    local module = link.entry_dialogue_module or ""
    if module == "" or module == "-" then return {} end

    local function_name = link.entry_dialogue_function or ""
    local location_type = link.entry_location_type or ""
    local location_id = link.entry_location_id or ""
    assert(function_name ~= "" and function_name ~= "-" and
           location_type ~= "" and location_type ~= "-" and
           location_id ~= "" and location_id ~= "-",
        "location entry dialogue has incomplete task/location data")

    local alternate = link.entry_alternate_result
    if alternate ~= nil and alternate ~= "" and alternate ~= "-" then
        local parsed = tonumber(alternate)
        assert(parsed ~= nil and parsed % 1 == 0,
            "location entry alternate result must be an integer")
        if context.alternate_result ~= parsed then return {} end
    end
    if not conditions_match(link.entry_conditions, context) then return {} end

    local text_profile = link.entry_text_profile or ""
    local speaker_id = link.entry_speaker_id or ""
    local event_id = link.entry_event_id or ""
    local continue_result = link.entry_continue_result_id or ""
    if text_profile == "-" then text_profile = "" end
    if speaker_id == "-" then speaker_id = "" end
    if event_id == "-" then event_id = "" end
    if continue_result == "-" then continue_result = "" end
    return {
        entry_dialogue_module = module,
        entry_dialogue_function = function_name,
        entry_location_type = location_type,
        entry_location_id = location_id,
        entry_text_profile = text_profile,
        entry_speaker_id = speaker_id,
        entry_event_id = event_id,
        entry_continue_result_id = continue_result,
    }
end

local function add_entry_dialogue(route, link, context)
    for name, value in pairs(entry_dialogue_fields(link, context)) do
        route[name] = value
    end
    return route
end

local function location_script_task(link, prefix)
    local module = link[prefix .. "_module"] or ""
    if module == "" or module == "-" then return nil end

    local function_name = link[prefix .. "_function"] or ""
    local location_type = link.location_type or ""
    local location_id = link.location_id or ""
    local text_profile = link.location_text_profile or ""
    local speaker_id = link.location_speaker_id or ""
    assert(function_name ~= "" and function_name ~= "-" and
           location_type ~= "" and location_type ~= "-" and
           location_id ~= "" and location_id ~= "-" and
           text_profile ~= "" and text_profile ~= "-" and
           speaker_id ~= "" and speaker_id ~= "-",
        "location script task has incomplete module/function/context data")
    return {
        module = module,
        function_name = function_name,
        location_type = location_type,
        location_id = location_id,
        text_profile = text_profile,
        speaker_id = speaker_id,
    }
end

local function add_location_script_tasks(route, link)
    route.indexed_title_service =
        location_script_task(link, "indexed_title_service")
    route.field_exit_prompt = location_script_task(link, "field_exit_prompt")
    return route
end

local function area_event_source_matches(link, context)
    if link.route_kind == route_kind.area_event then
        return context.event_table_available == true and
            type(context.event_table) == "table" and
            context.event_index < #context.event_table and
            context.event_table[context.event_index + 1] ==
                link.destination_scene
    end
    if link.route_kind ~= route_kind.area_event_override then
        return false
    end

    local source_destination = link.source_table_destination
    if type(source_destination) ~= "number" or
       source_destination < 0 or source_destination > 255 or
       source_destination % 1 ~= 0 then
        return false
    end
    if context.event_table_available == true and
       type(context.event_table) == "table" and
       context.event_index < #context.event_table then
        return context.event_table[context.event_index + 1] ==
            source_destination
    end

    -- Some ROM profiles have a null event-table pointer. The one-time source
    -- audit may bind the resulting event-0 fallback as a static route; the
    -- runtime does not reproduce null-pointer address arithmetic.
    return context.event_table_available == false and
        context.event_index == 0 and source_destination == 0
end

function Building.resolve_field_location_route(context)
    if type(context) ~= "table" or
       type(context.building_interaction_mode) ~= "boolean" or
       type(context.scene_id) ~= "number" or
       context.scene_id % 1 ~= 0 or context.scene_id < 0 or
       context.scene_id > 255 or
       type(context.event_code) ~= "string" or
       type(context.event_index) ~= "number" or
       context.event_index < 0 or context.event_index > 255 or
       type(context.alternate_result) ~= "number" then
        return no_route("event_0")
    end

    local link = selected_link(World.location_links_for_event(
        context.scene_id, context.event_code), context)
    if link ~= nil then
        -- Entity-owned Lua link data describes the interaction. Ordinary
        -- area destinations are accepted only when they agree with the
        -- current map's imported ROM event-table byte.
        if link.route_kind == route_kind.dialogue and
           link.script == route_script.lua_dialogue then
            local route = add_entry_dialogue({
                kind = route_kind.dialogue,
                script = route_script.lua_dialogue,
                event_code = context.event_code,
            }, link, context)
            assert(route.entry_dialogue_module ~= nil,
                "Lua dialogue link must provide a gated dialogue task")
            local return_policy = return_fields(link)
            route.return_anchor = return_policy.return_anchor
            route.return_facing = return_policy.return_facing
            return route
        elseif link.route_kind == route_kind.field_service and
               link.script == route_script.host_field_service then
            local service_id = link.field_service_id or ""
            assert(service_id ~= "" and service_id ~= "-",
                "host field-service link must name its native service")
            return {
                kind = route_kind.field_service,
                script = link.script,
                event_code = context.event_code,
                field_service_id = service_id,
            }
        elseif link.route_kind == route_kind.field_service and
               link.script == "lua_field_service" then
            local service = location_script_task(
                link, "indexed_title_service")
            assert(service ~= nil,
                "Lua field-service link must provide an indexed service task")
            return {
                kind = route_kind.field_service,
                script = link.script,
                event_code = context.event_code,
                indexed_title_service = service,
            }
        elseif (link.route_kind == route_kind.area_event or
                link.route_kind == route_kind.area_event_override) and
               link.script == route_script.map_event_table and
               type(link.destination_scene) == "number" and
               link.destination_scene >= 0 and
               link.destination_scene <= 255 and
               link.destination_scene % 1 == 0 and
               area_event_source_matches(link, context) then
            local return_policy = return_fields(link)
            local mode = link.destination_mode
            if mode ~= destination_mode.field and
               mode ~= destination_mode.resource_activity and
               mode ~= destination_mode.scripted_activity and
               mode ~= destination_mode.halted then
                mode = classify_area_event_destination(
                    link.destination_scene)
            end
            local route = add_entry_dialogue({
                kind = route_kind.area_event,
                script = route_script.map_event_table,
                event_code = context.event_code,
                area_event_target = link.destination_scene,
                area_event_destination_mode = mode,
                return_anchor = return_policy.return_anchor,
                return_facing = return_policy.return_facing,
            }, link, context)
            return add_location_script_tasks(route, link)
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

-- Location text, speaker properties, and conditional building content are
-- selected by the active location profile. Keep the operation generic so a
-- building's ROM-backed entry page is not hardcoded into the renderer.
function Building.present_entry_description()
    if game.location.type ~= "building" or
       game.location.id == nil then
        return "skip"
    end
    game.ui.say("entry")
    return "shown"
end

-- Elevator trigger scenes and their child scene lists are supplied from the
-- extracted US ROM records. The script owns the entry action and floor list.
-- A floor index may preview a destination, but confirmed travel uses the
-- current floor, which changes only after the A-button lift animation.
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
       type(context.floor_index) ~= "number" or
       context.floor_index % 1 ~= 0 or
       context.floor_index < 0 or
       context.floor_index >= #context.floor_scenes then
        return nil
    end

    local destination = context.floor_scenes[context.floor_index + 1]
    if type(destination) ~= "number" or destination % 1 ~= 0 or
       destination < 0 or destination > 255 then
        return nil
    end
    return destination
end

return Building
