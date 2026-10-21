package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")

local base_link = {
    source_scene = 1,
    source_event = "event_7",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 13,
    destination_mode = "field",
    route_priority = 0,
    return_anchor = "collision_probe_lead",
    return_facing = "face_away_from_entry",
}
local gated_link = {
    source_scene = 1,
    source_event = "event_7",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 13,
    destination_mode = "field",
    route_priority = 5,
    return_anchor = "collision_probe_lead",
    return_facing = "face_away_from_entry",
    route_conditions =
        "story_flag_bit_30=true;active_run=package_recovery",
    entry_conditions = "story_flag_bit_21=false",
    entry_dialogue_module = "building",
    entry_dialogue_function = "present_entry_description",
    entry_location_type = "building",
    entry_location_id = "source_test_location",
    entry_text_profile = "source.test.entry",
    entry_speaker_id = "source_test_speaker",
    entry_continue_result_id = "continue",
}

game = {
    state = {
        story_flag_bit_30 = true,
        story_flag_bit_21 = false,
    },
    active_run = "package_recovery",
    current_run = "employee_recruitment",
    runs = {
        registry = {
            active_run_id = function() return game.active_run end,
            current_offer_id = function() return game.current_run end,
        },
    },
    modules = {
        world_content_data = {
            entities = {
                source_test = { links = { base_link, gated_link } },
            },
            areas = {
                [1] = { { entity_id = "source_test" } },
            },
        },
    },
}

local context = {
    scene_id = 1,
    building_interaction_mode = false,
    event_code = "event_7",
    event_index = 7,
    event_table_available = true,
    event_table = { 0, 0, 0, 0, 0, 0, 0, 13 },
    alternate_result = 0,
}

local gated = Building.resolve_field_location_route(context)
assert(gated.kind == "area_event" and gated.area_event_target == 13)
assert(gated.entry_dialogue_module == "building")
assert(gated.entry_dialogue_function == "present_entry_description")
assert(gated.entry_location_id == "source_test_location")
assert(gated.entry_text_profile == "source.test.entry")
assert(gated.entry_speaker_id == "source_test_speaker")
assert(gated.entry_continue_result_id == "continue")
assert(gated.return_anchor == "collision_probe_lead")
assert(gated.return_facing == "face_away_from_entry")

game.active_run = "none"
local no_active_run = Building.resolve_field_location_route(context)
assert(no_active_run.kind == "area_event" and
       no_active_run.area_event_target == 13)
assert(no_active_run.entry_dialogue_module == nil)

game.active_run = "package_recovery"
game.state.story_flag_bit_30 = false
local default_route = Building.resolve_field_location_route(context)
assert(default_route.kind == "area_event" and
       default_route.area_event_target == 13)
assert(default_route.entry_dialogue_module == nil)

game.state.story_flag_bit_30 = true
game.state.story_flag_bit_21 = true
gated_link.route_conditions = "current_run=employee_recruitment"
local repeat_visit = Building.resolve_field_location_route(context)
assert(repeat_visit.kind == "area_event" and
       repeat_visit.area_event_target == 13)
assert(repeat_visit.entry_dialogue_module == nil)

game.state.story_flag_bit_21 = false
gated_link.route_conditions =
    "story_flag_bit_30=true;active_run=package_recovery"
gated_link.route_priority = 0
local ambiguous = Building.resolve_field_location_route(context)
assert(ambiguous.kind == "area_event" and
       ambiguous.area_event_target == 13)
assert(ambiguous.entry_dialogue_module == nil)

local override_link = {
    source_scene = 1,
    source_event = "event_7",
    route_kind = "area_event_override",
    script = "area_event_table",
    destination_scene = 54,
    destination_mode = "scripted_activity",
    source_table_destination = 13,
    route_priority = 10,
    return_anchor = "collision_probe_lead",
    return_facing = "face_away_from_entry",
    route_conditions =
        "story_flag_bit_30=true;story_flag_bit_21=false;alternate_result=0",
    entry_conditions = "story_flag_bit_30=true;story_flag_bit_21=false",
    entry_alternate_result = 0,
    entry_dialogue_module = "tarislar_apartments",
    entry_dialogue_function = "present_first_entry",
    entry_location_type = "building",
    entry_location_id = "puyallup_site_02_tarislar_garden_apartments",
    entry_text_profile = "tarislar.garden.apartments.entry",
    entry_speaker_id = "tarislar_apartments_narrator",
    entry_continue_result_id = "enter_scene_54",
}
game.state.story_flag_bit_30 = true
game.state.story_flag_bit_21 = false
game.modules.world_content_data.entities.source_test.links = {
    base_link, override_link,
}
context.alternate_result = 0
local first_entry = Building.resolve_field_location_route(context)
assert(first_entry.kind == "area_event" and
       first_entry.area_event_target == 54 and
       first_entry.area_event_destination_mode == "scripted_activity" and
       first_entry.entry_dialogue_module == "tarislar_apartments" and
       first_entry.return_anchor == "collision_probe_lead" and
       first_entry.return_facing == "face_away_from_entry" and
       first_entry.entry_continue_result_id == "enter_scene_54")

local tomb_link = {
    source_scene = 1,
    source_event = "event_7",
    route_kind = "area_event_override",
    script = "area_event_table",
    destination_scene = 54,
    destination_mode = "scripted_activity",
    source_table_destination = 13,
    route_priority = 10,
    route_conditions =
        "story_flag_bit_30=true;story_flag_bit_21=true;" ..
        "story_flag_bit_16=true;story_flag_bit_28=true;" ..
        "story_flag_bit_26=true;story_flag_bit_27=true;" ..
        "alternate_result=0",
    entry_conditions =
        "story_flag_bit_30=true;story_flag_bit_21=true;" ..
        "story_flag_bit_16=true;story_flag_bit_28=true;" ..
        "story_flag_bit_26=true;story_flag_bit_27=true",
    entry_alternate_result = 0,
    entry_dialogue_module = "tarislar_apartments",
    entry_dialogue_function = "present_tomb_departure",
    entry_location_type = "building",
    entry_location_id = "puyallup_site_02_tarislar_garden_apartments",
    entry_text_profile = "tarislar.garden.apartments.entry",
    entry_speaker_id = "tarislar_apartments_narrator",
    entry_continue_result_id = "enter_scene_54",
}
for _, bit in ipairs({ 16, 21, 26, 27, 28, 30 }) do
    game.state["story_flag_bit_" .. bit] = true
end
game.modules.world_content_data.entities.source_test.links = {
    base_link, tomb_link,
}
local tomb_departure = Building.resolve_field_location_route(context)
assert(tomb_departure.kind == "area_event" and
       tomb_departure.area_event_target == 54 and
       tomb_departure.entry_dialogue_function == "present_tomb_departure" and
       tomb_departure.entry_continue_result_id == "enter_scene_54")
for _, bit in ipairs({ 16, 21, 26, 27, 28, 30 }) do
    game.state["story_flag_bit_" .. bit] = false
    local denied_tomb_route = Building.resolve_field_location_route(context)
    assert(denied_tomb_route.kind == "area_event" and
           denied_tomb_route.area_event_target == 13 and
           denied_tomb_route.entry_dialogue_module == nil)
    game.state["story_flag_bit_" .. bit] = true
end
context.alternate_result = 1
local alternate_tomb_route = Building.resolve_field_location_route(context)
assert(alternate_tomb_route.kind == "area_event" and
       alternate_tomb_route.area_event_target == 13 and
       alternate_tomb_route.entry_dialogue_module == nil)
context.alternate_result = 0

local guarded_link = {
    source_scene = 1,
    source_event = "event_7",
    route_kind = "area_event_override",
    script = "area_event_table",
    destination_scene = 13,
    destination_mode = "field",
    source_table_destination = 13,
    route_priority = 10,
    route_conditions = "story_flag_bit_30=false;alternate_result=0",
    entry_conditions = "story_flag_bit_30=false",
    entry_alternate_result = 0,
    entry_dialogue_module = "tarislar_apartments",
    entry_dialogue_function = "present_guarded_entry",
    entry_location_type = "building",
    entry_location_id = "puyallup_site_02_tarislar_garden_apartments",
    entry_text_profile = "tarislar.garden.apartments.entry",
    entry_speaker_id = "tarislar_apartments_narrator",
    entry_continue_result_id = "enter_scene_13",
}
game.state.story_flag_bit_30 = false
game.modules.world_content_data.entities.source_test.links = {
    base_link, guarded_link,
}
local guarded_visit = Building.resolve_field_location_route(context)
assert(guarded_visit.kind == "area_event" and
       guarded_visit.area_event_target == 13 and
       guarded_visit.entry_dialogue_function == "present_guarded_entry" and
       guarded_visit.entry_continue_result_id == "enter_scene_13")

context.alternate_result = -1
local alternate_result = Building.resolve_field_location_route(context)
assert(alternate_result.kind == "area_event" and
       alternate_result.area_event_target == 13 and
       alternate_result.entry_dialogue_module == nil)
context.alternate_result = 0
context.event_table[8] = 12
local source_table_mismatch = Building.resolve_field_location_route(context)
assert(source_table_mismatch.kind == "area_event" and
       source_table_mismatch.area_event_target == 12 and
       source_table_mismatch.entry_dialogue_module == nil)

-- The Inn keeps the ROM's event-table byte as route evidence while its
-- entity-owned Lua tasks handle the indexed title service and generic prompt.
local inn_context = {
    scene_id = 1,
    building_interaction_mode = false,
    event_code = "event_1",
    event_index = 1,
    event_table_available = true,
    event_table = { 0, 53 },
    alternate_result = 0,
}
local inn_location = {
    location_type = "building",
    location_id = "puyallup_site_03_tarislar_city_inn",
    location_text_profile = "tarislar.city_inn",
    location_speaker_id = "tarislar_city_inn_narrator",
}
game.modules.world_content_data.entities.source_test.links = {
    {
        source_scene = 1,
        source_event = "event_1",
        route_kind = "area_event",
        script = "area_event_table",
        destination_scene = 53,
        destination_mode = "scripted_activity",
        route_priority = 0,
        field_exit_prompt_module = "tarislar_city_inn",
        field_exit_prompt_function = "present_field_exit_prompt",
        location_type = inn_location.location_type,
        location_id = inn_location.location_id,
        location_text_profile = inn_location.location_text_profile,
        location_speaker_id = inn_location.location_speaker_id,
    },
    {
        source_scene = 1,
        source_event = "event_1",
        route_kind = "field_service",
        script = "lua_field_service",
        route_priority = 10,
        route_conditions =
            "story_flag_bit_0=true;story_flag_bit_1=false;alternate_result=0",
        indexed_title_service_module = "tarislar_city_inn",
        indexed_title_service_function = "present_indexed_title_service",
        location_type = inn_location.location_type,
        location_id = inn_location.location_id,
        location_text_profile = inn_location.location_text_profile,
        location_speaker_id = inn_location.location_speaker_id,
    },
}
game.state.story_flag_bit_0 = true
game.state.story_flag_bit_1 = false
local inn_service = Building.resolve_field_location_route(inn_context)
assert(inn_service.kind == "field_service" and
       inn_service.script == "lua_field_service" and
       inn_service.indexed_title_service.module == "tarislar_city_inn" and
       inn_service.indexed_title_service.function_name ==
           "present_indexed_title_service" and
       inn_service.indexed_title_service.location_id ==
           inn_location.location_id)

game.state.story_flag_bit_1 = true
local inn_generic_prompt = Building.resolve_field_location_route(inn_context)
assert(inn_generic_prompt.kind == "area_event" and
       inn_generic_prompt.area_event_target == 53 and
       inn_generic_prompt.field_exit_prompt.module == "tarislar_city_inn" and
       inn_generic_prompt.field_exit_prompt.function_name ==
           "present_field_exit_prompt" and
       inn_generic_prompt.field_exit_prompt.text_profile ==
           "tarislar.city_inn")

inn_context.alternate_result = -1
local inn_package_result = Building.resolve_field_location_route(inn_context)
assert(inn_package_result.kind == "area_event" and
       inn_package_result.field_exit_prompt.function_name ==
           "present_field_exit_prompt")

-- Same-map building interactions use an entity-owned Lua task and preserve
-- the ROM door-return policy without a C++ building-specific dispatch.
local motel_context = {
    scene_id = 2,
    building_interaction_mode = true,
    event_code = "event_2",
    event_index = 2,
    event_table_available = true,
    event_table = { 20, 26, 26 },
    alternate_result = 0,
}
local motel_link = {
    source_scene = 2,
    source_event = "event_2",
    route_kind = "dialogue",
    script = "lua_dialogue",
    route_priority = 0,
    route_conditions = "story_flag_bit_0=false;alternate_result=0",
    requires_mode = "building_interaction",
    return_anchor = "collision_probe_lead",
    return_facing = "face_away_from_entry",
    entry_conditions = "story_flag_bit_0=false;alternate_result=0",
    entry_dialogue_module = "michael_package",
    entry_dialogue_function = "stoker_interaction",
    entry_location_type = "motel",
    entry_location_id = "stoker_motel",
    entry_text_profile = "stoker.motel",
    entry_speaker_id = "stoker",
}
game.modules.world_content_data.entities.source_test.links = { motel_link }
game.modules.world_content_data.areas[2] = {
    { entity_id = "source_test" },
}
game.state.story_flag_bit_0 = false
local motel_unpaid = Building.resolve_field_location_route(motel_context)
assert(motel_unpaid.kind == "dialogue" and
       motel_unpaid.script == "lua_dialogue" and
       motel_unpaid.area_event_target == nil and
       motel_unpaid.entry_dialogue_module == "michael_package" and
       motel_unpaid.entry_dialogue_function == "stoker_interaction" and
       motel_unpaid.return_anchor == "collision_probe_lead" and
       motel_unpaid.return_facing == "face_away_from_entry")

-- Same-map contacts carry their ROM-selected dialogue event through the
-- generic route task, without a scene-number dispatch in the host.
local restaurant_contact = {
    source_scene = 10,
    source_event = "event_30",
    route_kind = "dialogue",
    script = "lua_dialogue",
    route_priority = 0,
    entry_dialogue_module = "field_events",
    entry_dialogue_function = "present_location_dialogue",
    entry_location_type = "building",
    entry_location_id = "council_friendship_restaurant",
    entry_text_profile = "jackals_lantern.old_man",
    entry_speaker_id = "jackals_lantern_old_man",
    entry_event_id = "opening",
}
game.modules.world_content_data.entities.source_test.links = {
    restaurant_contact,
}
game.modules.world_content_data.areas[10] = {
    { entity_id = "source_test" },
}
local restaurant_event = Building.resolve_field_location_route({
    scene_id = 10,
    building_interaction_mode = false,
    event_code = "event_30",
    event_index = 30,
    event_table_available = false,
    event_table = {},
    alternate_result = 0,
})
assert(restaurant_event.kind == "dialogue" and
       restaurant_event.entry_dialogue_module == "field_events" and
       restaurant_event.entry_dialogue_function ==
           "present_location_dialogue" and
       restaurant_event.entry_location_id ==
           "council_friendship_restaurant" and
       restaurant_event.entry_event_id == "opening")

-- Scene 15's event 30 is a separate contact in the shared ROM scene. The
-- default continuation is the same no-choice old-man passage, independent of
-- which exterior route loaded the map.
local scene15_contact = {
    source_scene = 15,
    source_event = "event_30",
    route_kind = "dialogue",
    script = "lua_dialogue",
    route_priority = 0,
    entry_dialogue_module = "field_events",
    entry_dialogue_function = "present_location_dialogue",
    entry_location_type = "building",
    entry_location_id = "scene15_unidentified_contact",
    entry_text_profile = "jackals_lantern.old_man",
    entry_speaker_id = "jackals_lantern_old_man",
    entry_event_id = "opening",
}
game.modules.world_content_data.entities.source_test.links = {
    scene15_contact,
}
game.modules.world_content_data.areas[15] = {
    { entity_id = "source_test" },
}
local scene15_event = Building.resolve_field_location_route({
    scene_id = 15,
    building_interaction_mode = false,
    event_code = "event_30",
    event_index = 30,
    event_table_available = false,
    event_table = {},
    alternate_result = 0,
})
assert(scene15_event.kind == "dialogue" and
       scene15_event.script == "lua_dialogue" and
       scene15_event.area_event_target == nil and
       scene15_event.entry_location_id == "scene15_unidentified_contact" and
       scene15_event.entry_text_profile == "jackals_lantern.old_man" and
       scene15_event.entry_speaker_id == "jackals_lantern_old_man" and
       scene15_event.entry_event_id == "opening")

game.state.story_flag_bit_0 = true
local motel_paid = Building.resolve_field_location_route(motel_context)
assert(motel_paid.kind == "area_event" and
       motel_paid.area_event_target == 26 and
       motel_paid.entry_dialogue_module == nil)

-- The Wanderer map's south-edge exit uses the ROM's null event-table
-- fallback. Lua resolves the source-audited route without reproducing its
-- null-pointer address calculation at runtime.
local wanderer_exit_link = {
    source_scene = 14,
    source_event = "event_0",
    route_kind = "area_event_override",
    script = "area_event_table",
    destination_scene = 0,
    destination_mode = "field",
    source_table_destination = 0,
    route_priority = 0,
}
game.modules.world_content_data.entities.source_test.links = {
    wanderer_exit_link,
}
game.modules.world_content_data.areas[14] = {
    { entity_id = "source_test" },
}
local wanderer_exit = Building.resolve_field_location_route({
    scene_id = 14,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = false,
    event_table = {},
    alternate_result = 0,
})
assert(wanderer_exit.kind == "area_event" and
       wanderer_exit.script == "area_event_table" and
       wanderer_exit.area_event_target == 0 and
       wanderer_exit.area_event_destination_mode == "field" and
       wanderer_exit.entry_dialogue_module == nil and
       wanderer_exit.return_anchor == nil)

print("building route tests passed: conditional variants and entry tasks")
