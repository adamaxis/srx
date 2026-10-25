package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local E = require("enums")
local Building = require("building")
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
local hospital_interaction = {
    id = "rom_event_00",
    source = { sceneId = 0, eventCode = "event_0" },
    route = {
        kind = "area_event",
        script = "area_event_table",
        priority = 0,
        sourceRoutePolicy = "resume_source_route",
        returnAnchor = "collision_probe_lead",
        returnFacing = "face_away_from_entry",
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
    source = { sceneId = 13, eventCode = "event_0" },
    route = {
        kind = "area_event_override",
        script = "area_event_table",
        sourceRoutePolicy = "suppress_source_route",
        priority = 0,
        destinationScene = 0,
        destinationMode = "field",
        sourceTableDestination = 0,
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
            },
            areas = {
                [0] = {
                    { instance_id = "hospital", entity_id = HOSPITAL_ID },
                },
                [13] = {
                    { instance_id = "shared_exit", entity_id = SHARED_EXIT_ID },
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
        },
        location_conditions = { schemaVersion = 1, conditions = {} },
    },
}

-- Seattle General record 52 publishes scene-0/event-0. Its table byte is 21,
-- but ROM 0x1F83C enters the same-map hospital handler; the linked Lua task
-- consumes that event before the area-event destination can load.
local D = WorldLinks.data()
local hospital_service = WorldLinks.service_at(0, 0)
local hospital_source_route = WorldLinks.event_destination(0, 0)
local hospital_site = hospital_sites[HOSPITAL_ID]
local hospital_route = Building.resolve_field_location_route({
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = true,
    event_table = { hospital_source_route },
    alternate_result = 0,
})
check(hospital_service ~= nil and
      hospital_service.kind == E.ServiceKind.HOSPITAL and
      hospital_service.handler == 0x01F83C,
      "Seattle General scene 0/event 0 uses ROM hospital handler 0x1F83C")
check(hospital_source_route == 0x15,
      "Seattle General event-table row 0 at 0x0D54B2 is diagnostic 0x15")
check(hospital_site ~= nil and hospital_site.mode == 0 and
      hospital_site.event == 0 and hospital_site.text_root == 0x0B and
      hospital_site.entry_text == "seattle_general_hospital.entry.default",
      "Seattle General uses the scene-0/event-0 0x1F830 prompt profile")
check(hospital_route.kind == "area_event" and
      hospital_route.area_event_target == 0x15 and
      hospital_route.field_exit_prompt ~= nil and
      hospital_route.field_exit_prompt.module == "council_island_hospital" and
      hospital_route.field_exit_prompt.function_name ==
          "present_field_exit_prompt" and
      hospital_route.field_exit_prompt.location_id == HOSPITAL_ID and
      hospital_route.return_anchor == "collision_probe_lead" and
      hospital_route.return_facing == "face_away_from_entry",
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

-- The reported Gunderson exit is the shared Scene-13 south-edge route. Its
-- null table pointer at 0x1B3C54 makes 0x167AE read ROM byte 0, so the existing
-- route to Downtown Scene 0 matches the source and must remain unchanged.
local gunderson_exit = Building.resolve_field_location_route({
    scene_id = 13,
    building_interaction_mode = false,
    event_code = "event_0",
    event_index = 0,
    event_table_available = false,
    event_table = {},
    alternate_result = 0,
})
local gunderson_json = read_file(
    "decompilation/gameplay/world/entities/" .. SHARED_EXIT_ID ..
    "/interactions.json")
check(WorldLinks.event_destination(13, 0) == nil and
      D.scenes[13].event_table_address == nil and
      gunderson_exit.kind == "area_event" and
      gunderson_exit.area_event_target == 0 and
      contains(gunderson_json, '"destinationScene": 0') and
      contains(gunderson_json, '"sourceTableDestination": 0') and
      contains(gunderson_json, "0x1B3C54") and
      contains(gunderson_json, "0x167AE"),
      "Scene-13 event 0 remains the ROM's static field-Scene-0 fallback")

print(string.format("world_link_location_tests: %d/%d", pass, total))
os.exit(pass == total and 0 or 1)
