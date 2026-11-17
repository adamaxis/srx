package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" ..
    "decompilation/gameplay/tests_lua/?.lua;" .. package.path

local Building = require("route_test_support").wrap(require("building"))
local DialogueFramework = require("dialogue_framework")
local E = require("enums")
local Json = require("json")
local entity_id = "penumbra_site_05_lone_star"

local function read_json(path)
    local file = assert(io.open(path, "rb"), "cannot read " .. path)
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end

local interaction_document = read_json(
    "decompilation/gameplay/world/entities/" .. entity_id ..
    "/interactions.json")
local speakers = read_json("decompilation/gameplay/dialogue/speakers.json")
local script_text = read_json(
    "decompilation/assets/dialogue/script-text.json").texts
local interaction
for _, row in ipairs(interaction_document.interactions) do
    if row.id == "rom_event_06" then interaction = row end
end
assert(interaction ~= nil, "scene 3/event 6 interaction is missing")

local profile = speakers.profiles.lone_star_station_narrator
assert(profile ~= nil, "Lone Star narration profile is missing")
local function event_text(event_id)
    local event = assert(profile.events[event_id],
        "missing Lone Star event " .. event_id)
    return assert(script_text[event.title_key],
        "missing Lone Star text " .. event.title_key)
end

-- Byte-exact page content. 0x80 is represented as `|` in script-text.json.
assert(event_text("entry") ==
    "<Lone Star, dedicated to keeping the peace in these violent times... " ..
    "for a price.||Lone Star officers mill about while a security camera " ..
    "scans everyone who enters.||This is the last place any shadowrunner " ..
    "wants to be!>", "entry page must match ROM text at 0x11561D")
assert(event_text("package_handoff") ==
    "<The desk sergeant beckons you over.>||Okay, kid, here's the goods." ..
    "||Lose them and I'll drop kick you to Tokyo.",
    "negative package page must match ROM text at 0x1158F8")
assert(event_text("package_payment") ==
    "<The desk sergeant grabs the package and opens it.||He seems happy " ..
    "with the contents and pays you your nuyen.||You promptly beat feet, " ..
    "not wanting to get too familiar with Lone Star.>",
    "positive package page must match ROM text at 0x115968")
assert(event_text("citizen_reply") ==
    "<The desk sergeant looks up from his terminal.>||Got a problem, " ..
    "citizen?||<Without anything prepared, you confess you do not.||You " ..
    "then decide to leave before you draw anymore unwanted suspicion.>",
    "below-threshold page must match ROM text at 0x1156F8")
assert(event_text("camera_arrest") ==
    "The camera freezes on you as the doors lock and an alert sounds.||" ..
    "Cops swarm on you and before you can react, you are thrown in jail." ..
    "||You are fined for various crimes committed in the presence of Lone " ..
    "Star officers, as recorded by their cyber-eyes.",
    "arrest page must match ROM text at 0x1157BD")
assert(event_text("release") == "A few days later, you are|released.",
    "zero-wallet release page must retain the ROM line break at 0x1158D4")

local heat, funds, current_scene
local speeches, fine_texts, exit_operations
game = {
    location = { type = "building", id = entity_id },
    field = { package_run_status = E.PackageRunStatus.NONE },
    modules = {
        world_content_data = {
            entities = { [entity_id] = { id = entity_id, links = {} } },
            areas = {
                [3] = { { instance_id = "lone_star", entity_id = entity_id } },
            },
        },
    },
    content = {
        entities = { [entity_id] = { interactions = interaction_document } },
        speakers = speakers,
    },
    ui = {
        say = function(event_id, speaker_id, text_profile)
            local event = assert(profile.events[event_id],
                "unexpected Lone Star page " .. tostring(event_id))
            assert(speaker_id == "lone_star_station_narrator")
            assert(text_profile == "penumbra.lone_star")
            speeches[#speeches + 1] = {
                event = event_id,
                text = assert(script_text[event.title_key]),
            }
            return true
        end,
        say_text = function(text, continuation)
            fine_texts[#fine_texts + 1] = {
                text = text,
                continuation = continuation,
            }
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            if domain == "story" then
                if operation == "get_heat_level" then
                    return "level_" .. tostring(heat)
                end
                local written_level = operation:match("^set_heat_level_(%d+)$")
                if written_level ~= nil then
                    heat = tonumber(written_level)
                    return "applied"
                end
            elseif domain == "dialogue_exit" then
                exit_operations[#exit_operations + 1] = operation
                return "applied"
            elseif domain == "field" and operation == "scene_reference" then
                return "scene_" .. tostring(current_scene)
            end
            error("unexpected game service: " .. domain .. "." .. operation)
        end,
    },
    nuyen = function() return funds end,
    set_nuyen = function(value) funds = value end,
}

local DialogueSource = E.DialogueSource.BUILDING_EXIT

local function reset_case(case_heat, case_funds, package_status)
    heat, funds, current_scene = case_heat, case_funds, 3
    game.field.package_run_status = package_status or E.PackageRunStatus.NONE
    speeches, fine_texts, exit_operations = {}, {}, {}
end

local function open_scene_3_event_6()
    local route = Building.resolve_field_location_route({
        scene_id = 3,
        building_interaction_mode = false,
        event_code = "event_6",
        event_index = 6,
        event_table_available = true,
        event_table = { 0, 0, 0, 0, 0, 0, 41 },
        package_run_status = game.field.package_run_status,
    })
    assert(route.kind == "area_event" and route.area_event_target == 41,
        "the imported source event-table destination must remain documented")
    local task = assert(route.field_exit_prompt,
        "scene 3/event 6 must be intercepted by the ROM site handler")
    assert(task.module == "lone_star_station" and
           task.function_name == "present_field_exit_prompt" and
           task.location_id == entity_id)

    -- This is the production task dispatch used by the host's building-exit
    -- dialogue lifecycle. A linked prompt cancels the pending area route.
    local result = DialogueFramework.open(
        DialogueSource, task.module, task.function_name)
    assert(result == "field_return", "Lone Star prompt must return to Scene 3")
    assert(current_scene == 3,
        "scene 3/event 6 must not load its generic scene-41 event target")
    for _, operation in ipairs(exit_operations) do
        assert(operation ~= "scene_change",
            "Lone Star prompt must not request a scene transition")
    end
    return result
end

local function expect_pages(...)
    local expected = { ... }
    assert(#speeches == #expected,
        string.format("expected %d dialogue pages, got %d",
            #expected, #speeches))
    for index, event_id in ipairs(expected) do
        assert(speeches[index].event == event_id,
            string.format("page %d: expected %s, got %s",
                index, event_id, tostring(speeches[index].event)))
    end
end

-- Heat 2: entry page (0x110F66) continues to command 0x66 and page 0x1118F2.
reset_case(2, 10000)
open_scene_3_event_6()
expect_pages("entry", "citizen_reply")
assert(heat == 2 and funds == 10000 and #fine_texts == 0)

-- Heat 3: threshold page 0x1118FA continues to command 0x67; the 7500 fine
-- clears heat and debits the 10000-nuyen wallet to 2500.
reset_case(3, 10000)
open_scene_3_event_6()
expect_pages("entry", "camera_arrest")
assert(#fine_texts == 1 and fine_texts[1].text ==
       "Your fine comes to|      $7500!||A few days later, you are|released." and
       fine_texts[1].continuation == false)
assert(heat == 0 and funds == 2500)

-- A nonzero short wallet is zeroed. ROM 0x22814 replaces D1 with the wallet
-- before the number renderer at 0xC216, so the message displays $5000 here.
reset_case(4, 5000)
open_scene_3_event_6()
expect_pages("entry", "camera_arrest")
assert(#fine_texts == 1 and fine_texts[1].text ==
       "Your fine comes to|      $5000!||A few days later, you are|released.")
assert(heat == 0 and funds == 0)

-- Zero wallet selects page 0x11190E and returns before the fine display.
reset_case(3, 0)
open_scene_3_event_6()
expect_pages("entry", "camera_arrest", "release")
assert(heat == 0 and funds == 0 and #fine_texts == 0)

-- Signed FFF0D5 results bypass the entry/arrest chain and preserve package
-- result pages from 0x110F7A and 0x110F82.
reset_case(3, 10000, E.PackageRunStatus.HANDOFF_DUE)
open_scene_3_event_6()
expect_pages("package_handoff")
assert(heat == 3 and funds == 10000 and #fine_texts == 0)

reset_case(3, 10000, E.PackageRunStatus.PAYMENT_DUE)
open_scene_3_event_6()
expect_pages("package_payment")
assert(heat == 3 and funds == 10000 and #fine_texts == 0)

print("lone_star_station_tests=passed route=scene3/event6 heat=2,3,4 package_statuses=both")
