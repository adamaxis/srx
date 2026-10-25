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
    source_scene = 3,
    source_event = "event_2",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 21,
    destination_mode = "resource_activity",
    route_priority = 0,
}
local gated_entry = {
    source_scene = 3,
    source_event = "event_2",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 21,
    destination_mode = "resource_activity",
    route_priority = 10,
    route_condition_id = condition("test.fuchi.route", {
        equal("story.story_flag_bit_25", true),
        equal("story.story_flag_bit_14", false),
        equal("field.alternate_result", 0),
    }),
    entry_condition_id = condition("test.fuchi.entry", {
        equal("story.story_flag_bit_25", true),
        equal("story.story_flag_bit_14", false),
        equal("field.alternate_result", 0),
    }),
    entry_dialogue_module = "fuchi_offices",
    entry_dialogue_function = "present_cyberheart_entry",
    entry_location_type = "building",
    entry_location_id = "penumbra_site_02_fuchi",
    entry_text_profile = "penumbra.fuchi",
    entry_speaker_id = "fuchi_offices_narrator",
    entry_continue_outcome_type = "resume_source_route",
}

local story_bytes = {}
game = {
    state = { story_flag_bit_25 = true, story_flag_bit_14 = false },
    location = {
        type = "building", id = "penumbra_site_02_fuchi", scene_id = 3,
    },
    field = { mode = 2, event_code = 2, alternate_result = 0 },
    services = { call = function(domain, operation)
        if domain == "field" and operation == "scene_reference" then
            return "scene_3"
        end
        assert(domain == "story", "unexpected service " .. domain .. "." .. operation)
        local address = operation:match("^byte_get_(%x+)$")
        if address then
            local value = story_bytes[tonumber(address, 16)]
            return value ~= nil and ("value_" .. value) or nil
        end
        local address_text, value_text =
            operation:match("^byte_set_(%x+)_(%d+)$")
        if address_text then
            story_bytes[tonumber(address_text, 16)] = tonumber(value_text)
            return "applied"
        end
        error("unexpected story service " .. operation)
    end },
    ui = {
        say = function(event_id)
            game.last_dialogue_event = event_id
            return true
        end,
    },
    modules = {
        world_content_data = {
            entities = {
                penumbra_site_02_fuchi = {
                    links = { base_link, gated_entry },
                },
            },
            areas = {
                [3] = { { entity_id = "penumbra_site_02_fuchi" } },
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
game.content.conditions = load_dialogue_catalog("conditions")
game.content.cinematics = load_dialogue_catalog("cinematics")
game.content.conversations = {
    fuchi_offices = load_dialogue_catalog("conversations/fuchi_offices"),
}

local Dialogue = require("dialogue")
local FuchiOffices = require("fuchi_offices")

local context = {
    scene_id = 3,
    building_interaction_mode = false,
    event_code = "event_2",
    event_index = 2,
    event_table_available = true,
    event_table = { 0, 0, 21 },
    alternate_result = 0,
}

local route = Building.resolve_field_location_route(context)
assert(route.kind == "area_event" and route.area_event_target == 21)
assert(route.entry_dialogue_module == "fuchi_offices" and
       route.entry_dialogue_function == "present_cyberheart_entry" and
       route.entry_continue_outcome_type == "resume_source_route")
local entry_outcome = FuchiOffices.present_cyberheart_entry()
assert(entry_outcome.type == "resume_source_route")
assert(game.last_dialogue_event == "entry" and
       game.state.story_trigger_active_14 == true and
       game.state.story_flag_bit_14 == false and
       story_bytes[0xFFE187] == 0xFF)

local event = Dialogue.event_text_keys(
    "fuchi_offices_narrator", "penumbra.fuchi", "entry")
assert(event and event.title_key == "penumbra.fuchi.page.entry" and
       event.continuation_key == "shared.more")

game.state.story_flag_bit_14 = true
local completed_route = Building.resolve_field_location_route(context)
assert(completed_route.kind == "area_event" and
       completed_route.area_event_target == 21 and
       completed_route.entry_dialogue_module == nil)
local repeat_outcome = FuchiOffices.present_cyberheart_entry()
assert(repeat_outcome.type == "source_boundary" and
       repeat_outcome.reason:match("precondition:not_satisfied") ~= nil)

game.state.story_flag_bit_14 = false
game.state.story_flag_bit_25 = false
local pre_hospital_route = Building.resolve_field_location_route(context)
assert(pre_hospital_route.kind == "area_event" and
       pre_hospital_route.area_event_target == 21 and
       pre_hospital_route.entry_dialogue_module == nil)

game.state.story_flag_bit_25 = true
context.alternate_result = 1
local alternate_route = Building.resolve_field_location_route(context)
assert(alternate_route.kind == "area_event" and
       alternate_route.area_event_target == 21 and
       alternate_route.entry_dialogue_module == nil)

print("Fuchi offices tests passed: ROM gate, entry page, and activity route")
