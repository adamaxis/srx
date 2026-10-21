package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")
local Dialogue = require("dialogue")
local RenrakuOffices = require("renraku_offices")

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
    route_conditions =
        "story_flag_bit_16=true;story_flag_bit_12=false;alternate_result=0",
    entry_conditions = "story_flag_bit_16=true;story_flag_bit_12=false",
    entry_alternate_result = 0,
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
    ui = {
        say = function(event_id)
            game.last_dialogue_event = event_id
            return true
        end,
    },
}

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
assert(RenrakuOffices.present_map_overlay_entry() == "enter_scene_35")
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
assert(RenrakuOffices.present_map_overlay_entry() == "denied")

game.state.story_flag_bit_12 = false
game.state.story_flag_bit_16 = false
local pre_ito_route = Building.resolve_field_location_route(context)
assert(pre_ito_route.kind == "area_event" and
       pre_ito_route.area_event_target == 35 and
       pre_ito_route.entry_dialogue_module == nil)
assert(RenrakuOffices.present_map_overlay_entry() == "denied")

game.state.story_flag_bit_16 = true
context.alternate_result = 1
local alternate_route = Building.resolve_field_location_route(context)
assert(alternate_route.kind == "area_event" and
       alternate_route.area_event_target == 35 and
       alternate_route.entry_dialogue_module == nil)

print("Renraku Offices tests passed: ROM gate, entry page, and activity route")
