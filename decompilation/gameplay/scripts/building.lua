-- Generic field-location routing. Collision geometry and event-table bytes
-- are ROM-derived map data supplied by the native host; this module selects
-- the location interaction or the table destination without naming unknown
-- places or inventing an interior.
local Building = {}
local E = require("enums")
local World = require("world_content")
local resource_scene_entry_placements =
    require("data.resource_scene_entry_placements")
local ConditionEvaluator = require("condition_evaluator")
local condition_evaluator = ConditionEvaluator.new(
    ConditionEvaluator.FactProvider.new())
local validated_condition_catalog

local route_kind = {
    no_route = "no_route",
    dialogue = "dialogue",
    field_service = "field_service",
    area_event = "area_event",
    area_event_override = "area_event_override",
    source_activity_return = "source_activity_return",
    source_boundary = "source_boundary",
}

local route_script = {
    none = "none",
    map_event_table = "area_event_table",
    lua_dialogue = "lua_dialogue",
    host_field_service = "host_field_service",
    source_activity_return = "source_activity_return",
    source_boundary = "source_boundary",
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

local function attach_resource_scene_entry_placement(route)
    if route.kind ~= route_kind.area_event or
       route.area_event_destination_mode ~= destination_mode.resource_activity or
       route.event_code == "event_29" then
        return route
    end

    local scene_id = route.area_event_target
    if type(scene_id) ~= "number" or
       scene_id < E.Scene.RESOURCE_ACTIVITY_FIRST or
       scene_id > E.Scene.RESOURCE_ACTIVITY_LAST then
        return route
    end
    local placement = resource_scene_entry_placements[scene_id]
    assert(placement ~= nil,
        "resource scene entry placement is missing for scene " .. scene_id)
    route.resource_scene_entry_placement = placement
    return route
end

local function no_route(event_code)
    return {
        kind = route_kind.no_route,
        script = route_script.none,
        event_code = event_code,
    }
end

local function source_boundary(event_code, reason)
    return {
        kind = route_kind.source_boundary,
        script = route_script.source_boundary,
        event_code = event_code,
        boundary_reason = reason,
    }
end

local function condition_result(condition_id, context)
    if condition_id == nil or condition_id == "" or condition_id == "-" then
        return condition_evaluator:evaluate(nil, context)
    end
    assert(type(condition_id) == "string",
        "location condition reference must be a string")
    local content = type(game) == "table" and game.content or nil
    local catalog = type(content) == "table" and
        content.location_conditions or nil
    local definitions = type(catalog) == "table" and catalog.conditions or nil
    assert(type(catalog) == "table" and catalog.schemaVersion == 1 and
           type(definitions) == "table",
        "location condition catalog has an unsupported schema")
    if validated_condition_catalog ~= catalog then
        for id, expression in pairs(definitions) do
            assert(type(id) == "string" and id ~= "" and
                   type(expression) == "table",
                "location condition catalog has an invalid definition")
            ConditionEvaluator.validate(expression)
        end
        validated_condition_catalog = catalog
    end
    assert(type(definitions[condition_id]) == "table",
        "location link references an unknown condition: " .. condition_id)
    return condition_evaluator:evaluate(definitions[condition_id], context)
end

local function unresolved_condition_reason(link, scope, result)
    local link_identity = link.link_id or link.id or
        link[scope .. "_condition_id"] or "unnamed"
    local identity = (link.entity_id or "location") .. "." ..
        link_identity .. "." .. scope
    return "location.condition_unknown:" .. identity .. ":" ..
        table.concat(result.unknown_facts or {}, ",")
end

local function return_fields(link)
    local anchor = link.return_anchor
    local facing = link.return_facing
    return {
        return_anchor = anchor ~= "" and anchor or nil,
        return_facing = facing ~= "" and facing or nil,
    }
end

local function link_matches_context(link, context)
    return condition_result(link.route_condition_id, context)
end

local function selected_link(links, context)
    local selected
    local selected_priority
    local ambiguous = false
    local unresolved = {}
    for _, link in ipairs(links) do
        local evaluation = link_matches_context(link, context)
        local priority = link.route_priority or 0
        assert(type(priority) == "number" and priority % 1 == 0,
            "location link priority must be an integer")
        if evaluation.status == ConditionEvaluator.Status.unknown then
            unresolved[#unresolved + 1] = {
                priority = priority,
                reason = unresolved_condition_reason(
                    link, "route", evaluation),
            }
        elseif evaluation.status == ConditionEvaluator.Status.true_value then
            if selected == nil or priority > selected_priority then
                selected = link
                selected_priority = priority
                ambiguous = false
            elseif priority == selected_priority then
                ambiguous = true
            end
        end
    end
    if ambiguous then
        return nil, "location.route_ambiguous:" ..
            (selected.entity_id or "location") .. "." ..
            (selected.link_id or selected.id or "unnamed")
    end
    for _, candidate in ipairs(unresolved) do
        if selected == nil or candidate.priority >= selected_priority then
            return nil, candidate.reason
        end
    end
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

    local condition = condition_result(link.entry_condition_id, context)
    if condition.status == ConditionEvaluator.Status.false_value then
        return {}
    elseif condition.status == ConditionEvaluator.Status.unknown then
        return nil, unresolved_condition_reason(link, "entry", condition)
    end

    local text_profile = link.entry_text_profile or ""
    local speaker_id = link.entry_speaker_id or ""
    local event_id = link.entry_event_id or ""
    local continue_result = link.entry_continue_result_id or ""
    local continue_outcome = link.entry_continue_outcome_type or ""
    local resource_override = link.entry_resource_override or ""
    if text_profile == "-" then text_profile = "" end
    if speaker_id == "-" then speaker_id = "" end
    if event_id == "-" then event_id = "" end
    if continue_result == "-" then continue_result = "" end
    if continue_outcome == "-" then continue_outcome = "" end
    if resource_override == "-" then resource_override = "" end
    assert(continue_result == "" or continue_outcome == "",
        "location entry dialogue cannot use both result forms")
    assert(resource_override == "" or
           continue_result ~= "" or continue_outcome ~= "",
        "resource entry override needs a successful dialogue outcome")
    return {
        entry_dialogue_module = module,
        entry_dialogue_function = function_name,
        entry_location_type = location_type,
        entry_location_id = location_id,
        entry_text_profile = text_profile,
        entry_speaker_id = speaker_id,
        entry_event_id = event_id,
        entry_continue_result_id = continue_result,
        entry_continue_outcome_type = continue_outcome,
        entry_resource_override = resource_override,
    }
end

local function add_entry_dialogue(route, link, context)
    local fields, boundary_reason = entry_dialogue_fields(link, context)
    if boundary_reason ~= nil then
        return source_boundary(route.event_code, boundary_reason), true
    end
    for name, value in pairs(fields) do
        route[name] = value
    end
    return route, false
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
    route.route_preparation =
        location_script_task(link, "route_preparation")
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

local function source_event_table_destination(context)
    if context.event_table_available ~= true or
       type(context.event_table) ~= "table" or
       context.event_index >= #context.event_table then
        return nil
    end
    local destination = context.event_table[context.event_index + 1]
    if type(destination) ~= "number" or destination < 0 or
       destination > 255 or destination % 1 ~= 0 then
        return nil
    end
    return destination
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

    local link, unresolved_reason = selected_link(
        World.location_links_for_event(context.scene_id, context.event_code),
        context)
    if unresolved_reason ~= nil then
        return source_boundary(context.event_code, unresolved_reason)
    end
    if link ~= nil then
        -- Entity-owned Lua link data describes the interaction. Ordinary
        -- area destinations are accepted only when they agree with the
        -- current map's imported ROM event-table byte.
        if link.route_kind == route_kind.dialogue and
           link.script == route_script.lua_dialogue then
            local route, is_boundary = add_entry_dialogue({
                kind = route_kind.dialogue,
                script = route_script.lua_dialogue,
                event_code = context.event_code,
            }, link, context)
            if is_boundary then return route end
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
            if link.source_route_policy == "suppress_source_route" and
               not area_event_source_matches({
                   route_kind = route_kind.area_event_override,
                   source_table_destination =
                       link.source_table_destination,
               }, context) then
                return source_boundary(context.event_code,
                    "location.source_route_destination_mismatch:" ..
                    (link.entity_id or "location") .. "." ..
                    (link.link_id or link.id or "unnamed"))
            end
            assert(link.source_route_policy == nil or
                   link.source_route_policy == "suppress_source_route",
                "unsupported entity source-route policy: " ..
                    tostring(link.source_route_policy))
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
        elseif link.route_kind == route_kind.area_event and
               link.script == route_script.map_event_table and
               link.source_route_policy == "resume_source_route" then
            local target_scene = source_event_table_destination(context)
            if target_scene == nil then
                local has_source_task =
                    (link.entry_dialogue_module or "") ~= "" or
                    (link.route_preparation_module or "") ~= "" or
                    (link.indexed_title_service_module or "") ~= "" or
                    (link.field_exit_prompt_module or "") ~= ""
                if has_source_task then
                    return source_boundary(context.event_code,
                        "location.source_event_table_missing:" ..
                        (link.entity_id or "location") .. "." ..
                        (link.link_id or link.id or "unnamed"))
                end
                return no_route(context.event_code)
            end
            local return_policy = return_fields(link)
            local route, is_boundary = add_entry_dialogue({
                kind = route_kind.area_event,
                script = route_script.map_event_table,
                event_code = context.event_code,
                area_event_target = target_scene,
                area_event_destination_mode =
                    classify_area_event_destination(target_scene),
                return_anchor = return_policy.return_anchor,
                return_facing = return_policy.return_facing,
            }, link, context)
            if is_boundary then return route end
            return attach_resource_scene_entry_placement(
                add_location_script_tasks(route, link))
        elseif link.route_kind == route_kind.area_event and
               link.script == route_script.map_event_table and
               link.source_route_policy ~= nil then
            return source_boundary(context.event_code,
                "location.source_route_policy_unsupported:" ..
                (link.entity_id or "location") .. "." ..
                (link.link_id or link.id or "unnamed"))
        elseif link.route_kind == route_kind.area_event_override and
               link.script == route_script.map_event_table then
            local link_identity = (link.entity_id or "location") .. "." ..
                (link.link_id or link.id or "unnamed")
            if link.source_route_policy ~= "suppress_source_route" then
                local policy_failure = link.source_route_policy == nil and
                    "location.source_route_policy_missing:" or
                    "location.source_route_policy_unsupported:"
                return source_boundary(context.event_code,
                    policy_failure .. link_identity)
            end
            if not area_event_source_matches(link, context) then
                return source_boundary(context.event_code,
                    "location.source_route_destination_mismatch:" ..
                    link_identity)
            end
            if type(link.destination_scene) ~= "number" or
               link.destination_scene < 0 or
               link.destination_scene > 255 or
               link.destination_scene % 1 ~= 0 then
                return source_boundary(context.event_code,
                    "location.override_destination_invalid:" ..
                    link_identity)
            end
            local return_policy = return_fields(link)
            local mode = link.destination_mode
            if mode ~= destination_mode.field and
               mode ~= destination_mode.resource_activity and
               mode ~= destination_mode.scripted_activity and
               mode ~= destination_mode.halted then
                mode = classify_area_event_destination(
                    link.destination_scene)
            end
            local route, is_boundary = add_entry_dialogue({
                kind = route_kind.area_event,
                script = route_script.map_event_table,
                event_code = context.event_code,
                area_event_target = link.destination_scene,
                area_event_destination_mode = mode,
                return_anchor = return_policy.return_anchor,
                return_facing = return_policy.return_facing,
            }, link, context)
            if is_boundary then return route end
            return attach_resource_scene_entry_placement(
                add_location_script_tasks(route, link))
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
            local route, is_boundary = add_entry_dialogue({
                kind = route_kind.area_event,
                script = route_script.map_event_table,
                event_code = context.event_code,
                area_event_target = link.destination_scene,
                area_event_destination_mode = mode,
                return_anchor = return_policy.return_anchor,
                return_facing = return_policy.return_facing,
            }, link, context)
            if is_boundary then return route end
            return attach_resource_scene_entry_placement(
                add_location_script_tasks(route, link))
        elseif link.route_kind == route_kind.source_activity_return and
               link.script == route_script.source_activity_return then
            -- Scene 13/14 have null event-table pointers. Their event-0 exit
            -- follows the saved source field context through 0x2F32; only
            -- those two source-traced exits may select this authored route.
            if (context.scene_id ~= E.Scene.JUMP_HOUSE and
                context.scene_id ~= E.Scene.WANDERER) or
               context.event_code ~= "event_0" or
               context.event_index ~= 0 or
               context.event_table_available ~= false then
                return source_boundary(context.event_code,
                    "location.source_activity_return_context_mismatch:" ..
                    (link.entity_id or "location") .. "." ..
                    (link.link_id or link.id or "unnamed"))
            end
            return {
                kind = route_kind.source_activity_return,
                script = route_script.source_activity_return,
                event_code = context.event_code,
            }
        end
    end

    -- ROM resource scenes have no event table. Their zero-event collision
    -- starts the generic field transition, which returns through the saved
    -- source scene context rather than selecting a fixed map number.
    if context.scene_id >= 21 and context.scene_id <= 43 and
       context.event_code == "event_0" and
       context.event_table_available == false then
        return {
            kind = route_kind.source_activity_return,
            script = route_script.source_activity_return,
            event_code = context.event_code,
        }
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

    return attach_resource_scene_entry_placement({
        kind = route_kind.area_event,
        script = route_script.map_event_table,
        event_code = context.event_code,
        area_event_target = target_scene,
        area_event_destination_mode =
            classify_area_event_destination(target_scene),
    })
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
    if #group < 2 or #group > 6 then
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
