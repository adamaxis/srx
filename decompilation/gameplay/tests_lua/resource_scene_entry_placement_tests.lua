package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")
local E = require("enums")
local placements = require("data.resource_scene_entry_placements")

local profile_count = 0
for scene_id = E.Scene.RESOURCE_ACTIVITY_FIRST,
                 E.Scene.RESOURCE_ACTIVITY_LAST do
    local placement = placements[scene_id]
    assert(type(placement) == "table", "missing placement for scene " .. scene_id)
    assert(type(placement.camera_x) == "number" and
           type(placement.camera_y) == "number" and
           type(placement.runner_positions) == "table" and
           #placement.runner_positions == 3,
        "incomplete eight-word placement for scene " .. scene_id)
    for _, position in ipairs(placement.runner_positions) do
        assert(type(position.x) == "number" and type(position.y) == "number" and
               position.x >= 0 and position.x <= 65535 and
               position.y >= 0 and position.y <= 65535,
            "invalid runner position for scene " .. scene_id)
    end
    profile_count = profile_count + 1
end
assert(profile_count == 23)

game = {
    modules = { world_content_data = { entities = {}, areas = {} } },
    content = {},
}
local ordinary_entry = Building.resolve_field_location_route({
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = true,
    event_table = { E.Scene.RESOURCE_ACTIVITY_30 },
    alternate_result = 0,
})
assert(ordinary_entry.kind == "area_event" and
       ordinary_entry.area_event_target == E.Scene.RESOURCE_ACTIVITY_30 and
       ordinary_entry.resource_scene_entry_placement.camera_x == 0x1600 and
       ordinary_entry.resource_scene_entry_placement.camera_y == 0x4400 and
       ordinary_entry.resource_scene_entry_placement.runner_positions[1].x ==
           0x1F00 and
       ordinary_entry.resource_scene_entry_placement.runner_positions[1].y ==
           0x4900)

local elevator_event_table = {}
for index = 1, 30 do elevator_event_table[index] = 0 end
elevator_event_table[30] = E.Scene.RESOURCE_ACTIVITY_35
local elevator_entry = Building.resolve_field_location_route({
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_29",
    event_index = 29,
    event_table_available = true,
    event_table = elevator_event_table,
    alternate_result = 0,
})
assert(elevator_entry.kind == "area_event" and
       elevator_entry.area_event_target == E.Scene.RESOURCE_ACTIVITY_35 and
       elevator_entry.resource_scene_entry_placement == nil)

local lift_route = Building.resolve_field_location_entry({
    scene_id = E.Scene.RESOURCE_ACTIVITY_35,
    event_code = "event_29",
    source_elevator_groups = {
        [E.Scene.RESOURCE_ACTIVITY_35 + 1] = {
            E.Scene.RESOURCE_ACTIVITY_35,
            E.Scene.RESOURCE_ACTIVITY_36,
            E.Scene.RESOURCE_ACTIVITY_37,
            E.Scene.RESOURCE_ACTIVITY_38,
            E.Scene.RESOURCE_ACTIVITY_39,
        },
    },
})
assert(lift_route.action == "open_elevator_selector" and
       lift_route.entry_floor == 0 and
       lift_route.resource_scene_entry_placement == nil and
       Building.resolve_elevator_destination({
           floor_scenes = lift_route.floor_scenes,
           floor_index = 1,
       }) == E.Scene.RESOURCE_ACTIVITY_36)

print("Resource-scene entry placement tests passed: scenes 21-43 and lift route")
