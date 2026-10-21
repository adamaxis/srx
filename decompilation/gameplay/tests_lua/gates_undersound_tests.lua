package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")

local condition_definitions = {}
local function equal(fact, value)
    return { fact = fact, op = "eq", value = value }
end
local function condition(id, clauses)
    condition_definitions[id] = { all = clauses }
    return id
end

local base_link = {
    source_scene = 0,
    source_event = "event_3",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 21,
    destination_mode = "resource_activity",
    route_priority = 0,
}
local service_link = {
    source_scene = 0,
    source_event = "event_3",
    route_kind = "field_service",
    script = "lua_field_service",
    route_priority = 10,
    route_condition_id = condition("test.gates.route", {
        equal("story.story_flag_bit_11", true),
        equal("story.story_flag_bit_15", false),
        equal("field.alternate_result", 0),
    }),
    indexed_title_service_module = "gates_undersound",
    indexed_title_service_function = "present_ogami_departure",
    location_type = "building",
    location_id = "downtown_site_04_gates_undersound",
    location_text_profile = "gates.undersound",
    location_speaker_id = "gates_undersound_narrator",
}

game = {
    state = {
        story_flag_bit_11 = true,
        story_flag_bit_15 = false,
        field_mode_effect_flag_bit_26 = true,
        field_mode_effect_flag_bit_27 = false,
    },
    location = {
        type = "building",
        id = "downtown_site_04_gates_undersound",
    },
    field = { event_code = 3, alternate_result = 0 },
    ui = {
        say = function(event_id)
            game.last_dialogue_event = event_id
            return true
        end,
    },
    modules = {
        world_content_data = {
            entities = {
                downtown_site_04_gates_undersound = {
                    links = { base_link, service_link },
                },
            },
            areas = {
                [0] = {
                    { entity_id = "downtown_site_04_gates_undersound" },
                },
            },
        },
    },
    content = { location_conditions = {
        schemaVersion = 1, conditions = condition_definitions,
    } },
}

local Json = require("json")
local function load_dialogue_catalog(name)
    local file = assert(io.open(
        "decompilation/gameplay/dialogue/" .. name .. ".json", "rb"))
    local source = file:read("*a")
    file:close()
    return Json.decode(source)
end
game.content.speakers = load_dialogue_catalog("speakers")
game.content.contacts = load_dialogue_catalog("contacts")
game.content.menus = load_dialogue_catalog("menus")

local Dialogue = require("dialogue")
local GatesUndersound = require("gates_undersound")

local context = {
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_3",
    event_index = 3,
    event_table_available = true,
    event_table = { 0, 0, 0, 21 },
    alternate_result = 0,
}

local route = Building.resolve_field_location_route(context)
assert(route.kind == "field_service" and route.script == "lua_field_service")
assert(route.indexed_title_service and
       route.indexed_title_service.module == "gates_undersound" and
       route.indexed_title_service.function_name ==
           "present_ogami_departure")
assert(GatesUndersound.present_ogami_departure() == "field_return")
assert(game.last_dialogue_event == "entry" and
       game.state.story_flag_bit_15 == true and
       game.state.story_trigger_active_15 == true and
       game.state.field_mode_effect_flag_bit_26 == false and
       game.state.field_mode_effect_flag_bit_27 == true)

local event = Dialogue.event_text_keys(
    "gates_undersound_narrator", "gates.undersound", "entry")
assert(event and event.title_key == "gates.undersound.page.entry" and
       event.continuation_key == "")

game.state.story_flag_bit_15 = true
local repeat_route = Building.resolve_field_location_route(context)
assert(repeat_route.kind == "area_event" and
       repeat_route.area_event_target == 21 and
       repeat_route.indexed_title_service == nil)
assert(GatesUndersound.present_ogami_departure() == "denied")

game.state.story_flag_bit_15 = false
game.state.story_flag_bit_11 = false
local pre_mako_route = Building.resolve_field_location_route(context)
assert(pre_mako_route.kind == "area_event" and
       pre_mako_route.area_event_target == 21 and
       pre_mako_route.indexed_title_service == nil)

game.state.story_flag_bit_11 = true
context.alternate_result = 1
local alternate_route = Building.resolve_field_location_route(context)
assert(alternate_route.kind == "area_event" and
       alternate_route.area_event_target == 21 and
       alternate_route.indexed_title_service == nil)

print("Gates Undersound tests passed: ROM gate and same-map quest dialogue")
