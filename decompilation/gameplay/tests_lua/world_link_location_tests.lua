package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Building = require("route_test_support").wrap(require("building"))
local World = require("world_content")
local WorldLinks = require("world_links")
local hospital_sites = require("data.hospital_sites")

local pass, total = 0, 0
local function check(condition, label)
    total = total + 1
    if condition then
        pass = pass + 1
    else
        print("FAIL " .. label)
    end
end

local function read_file(path)
    local file = assert(io.open(path, "rb"), "cannot read " .. path)
    local contents = file:read("*a")
    file:close()
    return contents
end

local function contains(contents, needle)
    return contents:find(needle, 1, true) ~= nil
end

local HOSPITAL_ID = "downtown_site_12_seattle_general_hospital"
local SHARED_EXIT_ID = "shared_scene13_south_exit"
local WANDERER_EXIT_ID = "shared_scene14_south_exit"
local hospital_interaction = {
    id = "rom_event_00",
    source = { sceneId = 0, eventCode = "event_0" },
    route = {
        kind = "area_event",
        script = "area_event_table",
        priority = 0,
        sourceRoutePolicy = "resume_source_route",
    },
    tasks = {
        fieldExitPrompt = {
            module = "council_island_hospital",
            ["function"] = "present_field_exit_prompt",
        },
    },
    locationContext = {
        type = "building",
        id = HOSPITAL_ID,
        textProfile = "council_island_hospital",
        speakerId = "council_island_hospital_narrator",
    },
}
local shared_exit_interaction = {
    id = "rom_event_0",
    source = { sceneId = E.Scene.JUMP_HOUSE, eventCode = "event_0" },
    route = {
        kind = "source_activity_return",
        script = "source_activity_return",
        priority = 0,
    },
}
local wanderer_exit_interaction = {
    id = "rom_event_0",
    source = { sceneId = E.Scene.WANDERER, eventCode = "event_0" },
    route = {
        kind = "source_activity_return",
        script = "source_activity_return",
        priority = 0,
    },
}

local function interaction_document(entity_id, interaction)
    return {
        schemaVersion = 1,
        entityId = entity_id,
        interactions = { interaction },
    }
end

game = {
    state = {},
    active_run = "none",
    current_run = "none",
    runs = {
        registry = {
            active_run_id = function() return game.active_run end,
            current_offer_id = function() return game.current_run end,
        },
    },
    modules = {
        world_content_data = {
            entities = {
                [HOSPITAL_ID] = { id = HOSPITAL_ID, links = {} },
                [SHARED_EXIT_ID] = { id = SHARED_EXIT_ID, links = {} },
                [WANDERER_EXIT_ID] = { id = WANDERER_EXIT_ID, links = {} },
            },
            areas = {
                [0] = {
                    { instance_id = "hospital", entity_id = HOSPITAL_ID },
                },
                [E.Scene.JUMP_HOUSE] = {
                    { instance_id = "shared_exit", entity_id = SHARED_EXIT_ID },
                },
                [E.Scene.WANDERER] = {
                    { instance_id = "wanderer_exit",
                      entity_id = WANDERER_EXIT_ID },
                },
            },
        },
    },
    content = {
        entities = {
            [HOSPITAL_ID] = {
                interactions = interaction_document(
                    HOSPITAL_ID, hospital_interaction),
            },
            [SHARED_EXIT_ID] = {
                interactions = interaction_document(
                    SHARED_EXIT_ID, shared_exit_interaction),
            },
            [WANDERER_EXIT_ID] = {
                interactions = interaction_document(
                    WANDERER_EXIT_ID, wanderer_exit_interaction),
            },
        },
        location_conditions = { schemaVersion = 1, conditions = {} },
    },
}

-- Seattle General record 52 publishes scene-0/event-0. Its destination is 21,
-- but ROM 0x1F83C enters the same-map hospital handler; the linked Lua task
-- consumes that event before the area-event destination can load.
local D = WorldLinks.data()
local function has_contextual_transition(scene_id)
    for _, link in ipairs(WorldLinks.links_in_scene(scene_id)) do
        if link.transition and
           link.transition.kind == "contextual_transition" then
            return true
        end
    end
    return false
end
-- Scene-0 record 61 at 0x0D57E6 spans world pixels x=26624..27135,
-- y=13824..14335 and imports as the semantic hospital service doorway.
local hospital_service = WorldLinks.service_at(0, 26880, 14080)
-- This is a diagnostic source expectation, not runtime script data.
local hospital_source_route = 0x15
local hospital_site = hospital_sites[HOSPITAL_ID]
local hospital_route = Building.resolve_field_location_route({
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = true,
    event_table = { hospital_source_route },
    package_run_status = "none",
})
check(hospital_service ~= nil and
      hospital_service.kind == "hospital",
      "Seattle General scene 0/event_0 uses the hospital service policy")
check(hospital_source_route == 0x15,
      "Seattle General event-table row 0 at 0x0D54B2 is diagnostic 0x15")
check(hospital_site ~= nil and
      hospital_site.narrator == "council_island_hospital_narrator" and
      hospital_site.text_profile == "council_island_hospital" and
      hospital_site.entry_text == "seattle_general_hospital.entry.default",
      "Seattle General uses the scene-0/event-0 0x1F830 prompt profile")
check(hospital_route.kind == "area_event" and
      hospital_route.area_event_target == 0x15 and
      hospital_route.field_exit_prompt ~= nil and
      hospital_route.field_exit_prompt.module == "council_island_hospital" and
      hospital_route.field_exit_prompt.function_name ==
          "present_field_exit_prompt" and
      hospital_route.field_exit_prompt.location_id == HOSPITAL_ID and
      hospital_route.return_anchor ==
          E.FieldLocationReturnAnchor.COLLISION_PROBE_LEAD and
      hospital_route.return_facing ==
          E.FieldLocationReturnFacing.FACE_AWAY_FROM_ENTRY,
      "ROM 0x1F83C hospital prompt intercepts the table's Scene-21 route")

local hospital_json = read_file(
    "decompilation/gameplay/world/entities/" .. HOSPITAL_ID ..
    "/interactions.json")
check(contains(hospital_json, '"function": "present_field_exit_prompt"') and
      contains(hospital_json, '"id": "' .. HOSPITAL_ID .. '"') and
      contains(hospital_json, "0x0D54B2") and
      contains(hospital_json, "0x1F830") and
      contains(hospital_json, "0x1F83C") and
      contains(hospital_json, "0xC928"),
      "Seattle General interaction JSON records the ROM service intercept")
local script_text = read_file("decompilation/assets/dialogue/script-text.json")
check(contains(script_text, '"seattle_general_hospital.entry.default"') and
      contains(script_text, "Dr. Haversheen"),
      "Seattle General's 0x11199C entry page is imported as named text")

-- Scene-13 event 0 has a null event table, but the exit follows the saved
-- source-scene path through FFDF40 -> 0x00A72 -> 0x1E43C -> 0x2F32.
local gunderson_exit = Building.resolve_field_location_route({
    scene_id = E.Scene.JUMP_HOUSE,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = false,
    event_table = {},
    package_run_status = "none",
})
local gunderson_json = read_file(
    "decompilation/gameplay/world/entities/" .. SHARED_EXIT_ID ..
    "/interactions.json")
check(has_contextual_transition(13) and
      D.scenes[13].event_destinations == nil and
      gunderson_exit.kind == "source_activity_return" and
      gunderson_exit.script == "source_activity_return" and
      gunderson_exit.area_event_target == nil and
      contains(gunderson_json, '"kind": "source_activity_return"') and
      contains(gunderson_json, '"script": "source_activity_return"') and
      contains(gunderson_json, "0x1B3C54") and
      contains(gunderson_json, "FFDF40 -> 0x00A72 -> 0x1E43C -> 0x2F32"),
      "Scene-13 event 0 restores its source field snapshot")

local wanderer_exit = Building.resolve_field_location_route({
    scene_id = E.Scene.WANDERER,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = false,
    event_table = {},
    package_run_status = "none",
})
local wanderer_json = read_file(
    "decompilation/gameplay/world/entities/" .. WANDERER_EXIT_ID ..
    "/interactions.json")
check(has_contextual_transition(E.Scene.WANDERER) and
      D.scenes[E.Scene.WANDERER].event_destinations == nil and
      wanderer_exit.kind == "source_activity_return" and
      wanderer_exit.area_event_target == nil and
      contains(wanderer_json, '"kind": "source_activity_return"') and
      contains(wanderer_json, "0x1D5BA0") and
      contains(wanderer_json, "0x1D5BFC/0x1D5C06"),
      "Scene-14 event 0 restores its source field snapshot")

print(string.format("world_link_location_tests: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
