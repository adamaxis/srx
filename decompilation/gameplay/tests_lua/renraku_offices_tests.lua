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
    source_scene = 5,
    source_event = "event_6",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 35,
    destination_mode = "resource_activity",
    route_priority = 0,
}
local gated_entry = {
    source_scene = 5,
    source_event = "event_6",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 35,
    destination_mode = "resource_activity",
    route_priority = 10,
    route_condition_id = condition("test.renraku.route", {
        equal("story.story_flag_bit_16", true),
        equal("story.story_flag_bit_12", false),
        equal("field.alternate_result", 0),
    }),
    entry_condition_id = condition("test.renraku.entry", {
        equal("story.story_flag_bit_16", true),
        equal("story.story_flag_bit_12", false),
        equal("field.alternate_result", 0),
    }),
    entry_dialogue_module = "renraku_offices",
    entry_dialogue_function = "present_map_overlay_entry",
    entry_location_type = "building",
    entry_location_id = "renraku_offices_frontage",
    entry_text_profile = "renraku.offices",
    entry_speaker_id = "renraku_offices_narrator",
    entry_continue_result_id = "enter_scene_35",
}

game = {
    state = { story_flag_bit_16 = true, story_flag_bit_12 = false },
    location = {
        type = "building",
        id = "renraku_offices_frontage",
    },
    field = {
        mode = 1,
        event_code = 6,
        alternate_result = 0,
    },
    modules = {
        world_content_data = {
            entities = {
                renraku_offices_frontage = {
                    links = { base_link, gated_entry },
                },
            },
            areas = {
                [5] = {
                    { entity_id = "renraku_offices_frontage" },
                },
            },
        },
    },
    content = { location_conditions = {
        schemaVersion = 1, conditions = condition_definitions,
    } },
    ui = {
        say = function(event_id)
            game.last_dialogue_event = event_id
            return true
        end,
    },
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
game.content.conditions = load_dialogue_catalog("conditions")
game.content.cinematics = load_dialogue_catalog("cinematics")
game.content.conversations = {
    gates_undersound = load_dialogue_catalog(
        "conversations/gates_undersound"),
    renraku_offices = load_dialogue_catalog(
        "conversations/renraku_offices"),
}

local Dialogue = require("dialogue")
local RenrakuOffices = require("renraku_offices")

local context = {
    scene_id = 5,
    building_interaction_mode = false,
    event_code = "event_6",
    event_index = 6,
    event_table_available = true,
    event_table = { 0, 0, 0, 0, 0, 0, 35 },
    alternate_result = 0,
}

local route = Building.resolve_field_location_route(context)
assert(route.kind == "area_event" and route.area_event_target == 35)
assert(route.entry_dialogue_module == "renraku_offices" and
       route.entry_dialogue_function == "present_map_overlay_entry" and
       route.entry_continue_result_id == "enter_scene_35")
local entry_outcome = RenrakuOffices.present_map_overlay_entry()
assert(entry_outcome.type == "resume_source_route")
assert(game.last_dialogue_event == "entry" and
       game.state.story_trigger_active_12 == true and
       game.state.story_flag_bit_12 == false)

local event = Dialogue.event_text_keys(
    "renraku_offices_narrator", "renraku.offices", "entry")
assert(event and event.title_key == "renraku.offices.page.entry" and
       event.continuation_key == "shared.more")

game.state.story_flag_bit_12 = true
local repeat_route = Building.resolve_field_location_route(context)
assert(repeat_route.kind == "area_event" and
       repeat_route.area_event_target == 35 and
       repeat_route.entry_dialogue_module == nil)
local repeat_outcome = RenrakuOffices.present_map_overlay_entry()
assert(repeat_outcome.type == "source_boundary" and
       repeat_outcome.reason:match("precondition:not_satisfied") ~= nil)

game.state.story_flag_bit_12 = false
game.state.story_flag_bit_16 = false
local pre_ito_route = Building.resolve_field_location_route(context)
assert(pre_ito_route.kind == "area_event" and
       pre_ito_route.area_event_target == 35 and
       pre_ito_route.entry_dialogue_module == nil)
local pre_ito_outcome = RenrakuOffices.present_map_overlay_entry()
assert(pre_ito_outcome.type == "source_boundary" and
       pre_ito_outcome.reason:match("precondition:not_satisfied") ~= nil)

game.state.story_flag_bit_16 = true
context.alternate_result = 1
local alternate_route = Building.resolve_field_location_route(context)
assert(alternate_route.kind == "area_event" and
       alternate_route.area_event_target == 35 and
       alternate_route.entry_dialogue_module == nil)

print("Renraku Offices tests passed: ROM gate, entry page, and activity route")
