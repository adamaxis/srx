package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" ..
    "decompilation/gameplay/tests_lua/?.lua;" .. package.path

local Building = require("route_test_support").wrap(require("building"))
local DialogueFramework = require("dialogue_framework")
local E = require("enums")
local Json = require("json")
local DeckRules = require("menu.rules.deck")
local ENTITY_ID = "downtown_site_09_roscoe"
local FILE_SLOT_COUNT = DeckRules.FILES

local function read_json(path)
    local file = assert(io.open(path, "rb"), "cannot read " .. path)
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end

local interaction_document = read_json(
    "decompilation/gameplay/world/entities/" .. ENTITY_ID ..
    "/interactions.json")
local speakers_document = read_json("decompilation/gameplay/dialogue/speakers.json")
local script_text = read_json(
    "decompilation/assets/dialogue/script-text.json").texts
local speaker_id = "roscoe_the_fixer"
local text_profile = "downtown.roscoe"
local profile = speakers_document.profiles[speaker_id]
assert(profile ~= nil, "Roscoe speaker profile is missing")

local function event_text(event_id)
    local event = assert(profile.events[event_id],
        "missing Roscoe event " .. event_id)
    return assert(script_text[event.title_key],
        "missing Roscoe text " .. event.title_key)
end

assert(event_text("opening") ==
    "Howdy!  Name's Roscoe.||My biz is buying info and selling it off ta' " ..
    "the highest bidder.||Ya' got any data for me, pardner, and I'll gladly " ..
    "give ya' a fair deal.", "opening text must match ROM bytes at 0x11451B")
assert(event_text("connect_deck") ==
    "<He gestures to the terminal on his desk.>||Well, hook your deck on up " ..
    "to my ol' hawg here, and we'll get down ta' biz.",
    "deck connection text must match ROM bytes at 0x1145BD")
assert(event_text("farewell") ==
    "Adios, pardner!||Why'n't ya' go out and dredge up some datafiles for " ..
    "me ta' build up ma' inventory, huh?",
    "farewell text must match ROM bytes at 0x11488F")
assert(event_text("no_deck") ==
    "You don't even got a deck ta' store data on?!||I can't download stuff " ..
    "from your head... yet.  But I hear Mitsuhama's workin' on it!||'Til " ..
    "then, you an' I ain't got nothin' ta' talk about.",
    "no-deck text must match ROM bytes at 0x114635")
assert(event_text("encrypted") ==
    "This file here...|||It's encrypted with some|Johnson's personal " ..
    "code!!||I ain't even gonna' touch|this'un!",
    "encrypted text must match ROM bytes at 0x114824")
assert(event_text("useless") ==
    "This file here...|||It's useless!!||I'll delete it from your|storage, " ..
    "and I won't even|charge ya'!",
    "useless text must match ROM bytes at 0x1146F1")
assert(event_text("offer") ==
    "This file here...|||I might be able ta' find|a use for the info.||" ..
    "How's about I offer ya'|             $?",
    "offer template must match ROM bytes at 0x114754")
assert(event_text("end_of_file") ==
    "Looks like we reached the big End-O'-File marker.||Ya'll know ta' " ..
    "come ta' me when ya' get more data.",
    "end page must match ROM bytes at 0x1147BE")
assert(event_text("package_handoff") ==
    "<You tell him you are here to pick up a package.||He produces it, and " ..
    "hands it to you.>||Good day, sir.",
    "negative FFF0D5 page must match ROM bytes at 0x1149BE")
assert(event_text("package_payment") ==
    "<You give him the package.||He checks it to make sure it is authentic, " ..
    "then cheerfully pays you.>",
    "positive FFF0D5 page must match ROM bytes at 0x114A26")

local function empty_file()
    return { id = -1, size = 0, data_value = 0, run_objective = false }
end

local function make_record(overrides)
    local files = {}
    for slot = 0, FILE_SLOT_COUNT - 1 do
        files[slot] = empty_file()
    end
    for slot, file in pairs(overrides or {}) do
        files[slot] = file
    end
    return {
        files = files,
        active = { [-1] = -1, [0] = -1, [1] = -1, [2] = -1,
                   [3] = -1, [4] = -1 },
        ratings = {},
    }
end

local function make_slots()
    local slots = {}
    for slot = 0, FILE_SLOT_COUNT - 1 do
        slots[slot] = { status = 0, phase = 0, timer = 0 }
    end
    return slots
end

local current_scene, funds, has_deck, record, slots
local choice_queue, speeches, prompts, exit_operations, commit_count
local committed_files

local function next_choice(menu_id)
    assert(#choice_queue > 0, "unexpected prompt " .. tostring(menu_id))
    return table.remove(choice_queue, 1)
end

game = {
    location = { type = "building", id = ENTITY_ID },
    field = { package_run_status = E.PackageRunStatus.NONE },
    modules = {
        world_content_data = {
            entities = { [ENTITY_ID] = { id = ENTITY_ID, links = {} } },
            areas = { [0] = {
                { instance_id = "roscoe", entity_id = ENTITY_ID },
            } },
        },
    },
    content = {
        entities = { [ENTITY_ID] = { interactions = interaction_document } },
        speakers = speakers_document,
    },
    ui = {
        say = function(event_id, requested_speaker, requested_profile)
            assert(requested_speaker == speaker_id)
            assert(requested_profile == text_profile)
            speeches[#speeches + 1] = {
                event = event_id,
                text = event_text(event_id),
            }
            return true
        end,
        prompt_text_key = function(menu_id, text_key, choices,
                                   requested_speaker, requested_profile)
            assert(requested_speaker == speaker_id)
            assert(requested_profile == text_profile)
            prompts[#prompts + 1] = {
                menu = menu_id,
                text_key = text_key,
                text = assert(script_text[text_key]),
                choices = choices,
            }
            return next_choice(menu_id)
        end,
        prompt_text = function(menu_id, text, choices,
                               requested_speaker, requested_profile)
            assert(requested_speaker == speaker_id)
            assert(requested_profile == text_profile)
            prompts[#prompts + 1] = {
                menu = menu_id,
                text = text,
                choices = choices,
            }
            return next_choice(menu_id)
        end,
    },
    services = {
        call = function(domain, operation)
            if domain == "dialogue_exit" then
                exit_operations[#exit_operations + 1] = operation
                return "applied"
            elseif domain == "field" and operation == "scene_reference" then
                return "scene_" .. tostring(current_scene)
            end
            error("unexpected game service: " .. domain .. "." .. operation)
        end,
    },
    cyberdeck_available = function() return has_deck end,
    deck_record = function() return record end,
    deck_slots = function() return slots end,
    deck_commit = function(updated_record, updated_slots)
        assert(updated_record == record and updated_slots == slots)
        commit_count = commit_count + 1
        committed_files = {}
        for slot = 0, FILE_SLOT_COUNT - 1 do
            local file = record.files[slot]
            committed_files[slot] = {
                id = file.id,
                size = file.size,
                data_value = file.data_value,
                run_objective = file.run_objective,
            }
        end
    end,
    nuyen = function() return funds end,
    set_nuyen = function(value) funds = value end,
}

local function reset_case(case)
    current_scene = 0
    funds = case.funds or 100
    has_deck = case.has_deck == true
    record = make_record(case.files)
    slots = make_slots()
    game.field.package_run_status =
        case.package_status or E.PackageRunStatus.NONE
    choice_queue = case.choices or {}
    speeches, prompts, exit_operations = {}, {}, {}
    commit_count, committed_files = 0, nil
end

local function run_scene_0_event_7()
    local route = Building.resolve_field_location_route({
        scene_id = 0,
        building_interaction_mode = false,
        event_code = "event_7",
        event_index = 7,
        event_table_available = true,
        event_table = { 0, 0, 0, 0, 0, 0, 0, 21 },
        package_run_status = game.field.package_run_status,
    })
    assert(route.kind == "area_event" and route.area_event_target == 21,
        "the imported event-table target must remain documented")
    local task = assert(route.field_exit_prompt,
        "scene 0/event 7 must dispatch the Roscoe handler")
    assert(task.module == "roscoe_fixer" and
           task.function_name == "present_field_exit_prompt" and
           task.location_id == ENTITY_ID)

    local result = DialogueFramework.open(
        E.DialogueSource.BUILDING_EXIT, task.module, task.function_name)
    assert(result == "field_return",
        "Roscoe prompt must return to scene 0 without entering scene 21")
    assert(current_scene == 0,
        "scene 0/event 7 must not load its generic scene-21 event target")
    for _, operation in ipairs(exit_operations) do
        assert(operation ~= "scene_change",
            "Roscoe prompt must not request a scene transition")
    end
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

local function opening_prompt()
    local prompt = assert(prompts[1], "opening choice prompt is missing")
    assert(prompt.menu == "roscoe_opening" and
           prompt.text_key == "downtown.roscoe.opening")
    assert(prompt.choices[1].id == "sell" and
           prompt.choices[1].label == "I've got some data I'd like to sell.")
    assert(prompt.choices[2].id == "later" and
           prompt.choices[2].label == "I'll talk to you later, chum.")
    return prompt
end

reset_case({ funds = 500, has_deck = false, choices = { "sell" } })
run_scene_0_event_7()
opening_prompt()
expect_pages("connect_deck", "no_deck")
assert(funds == 500 and commit_count == 0)

reset_case({ has_deck = true, choices = { "sell" } })
run_scene_0_event_7()
opening_prompt()
expect_pages("connect_deck", "end_of_file")
assert(commit_count == 0)

reset_case({
    has_deck = true,
    choices = { "sell" },
    files = { [2] = { id = 0, size = 7, data_value = 0,
                      run_objective = false } },
})
run_scene_0_event_7()
expect_pages("connect_deck", "useless", "end_of_file")
assert(record.files[2].id == -1 and record.files[2].size == 0 and
       record.files[2].data_value == 0 and not record.files[2].run_objective)
assert(commit_count == 1 and committed_files[2].id == -1 and funds == 100)

reset_case({
    has_deck = true,
    choices = { "sell" },
    files = { [4] = { id = 3, size = 4, data_value = 0,
                      run_objective = true } },
})
run_scene_0_event_7()
expect_pages("connect_deck", "encrypted", "end_of_file")
assert(record.files[4].id == 3 and record.files[4].size == 4 and
       record.files[4].run_objective and funds == 100 and commit_count == 0)

reset_case({
    funds = 200,
    has_deck = true,
    choices = { "sell", "yes" },
    files = { [1] = { id = 24, size = 4, data_value = 3,
                      run_objective = false } },
})
run_scene_0_event_7()
expect_pages("connect_deck", "end_of_file")
assert(funds == 260 and commit_count == 1 and
       record.files[1].id == -1 and record.files[1].size == 0 and
       record.files[1].data_value == 0)
assert(prompts[2].text:find("Lone Star Reports", 1, true) and
       prompts[2].text:find("$60", 1, true),
       "offer must display the source file name and calculated price")
assert(prompts[2].choices[1].id == "yes" and
       prompts[2].choices[1].label == "Yes." and
       prompts[2].choices[2].id == "no" and
       prompts[2].choices[2].label == "No.")

reset_case({
    funds = 200,
    has_deck = true,
    choices = { "sell", "no" },
    files = { [1] = { id = 24, size = 4, data_value = 3,
                      run_objective = false } },
})
run_scene_0_event_7()
expect_pages("connect_deck", "end_of_file")
assert(funds == 200 and commit_count == 0 and
       record.files[1].id == 24 and record.files[1].size == 4 and
       record.files[1].data_value == 3)

reset_case({ choices = { "later" } })
run_scene_0_event_7()
opening_prompt()
expect_pages("farewell")

reset_case({ package_status = E.PackageRunStatus.HANDOFF_DUE })
run_scene_0_event_7()
expect_pages("package_handoff")
assert(#prompts == 0)

reset_case({ package_status = E.PackageRunStatus.PAYMENT_DUE })
run_scene_0_event_7()
expect_pages("package_payment")
assert(#prompts == 0)

print("roscoe_fixer_tests=passed route=scene0/event7 pages=datafile-sale statuses=both")

game = {
    location = { type = "building", id = ENTITY_ID },
    field = { package_run_status = E.PackageRunStatus.NONE },
    modules = {
        world_content_data = {
            entities = { [ENTITY_ID] = { id = ENTITY_ID, links = {} } },
            areas = { [0] = {
                { instance_id = "roscoe", entity_id = ENTITY_ID },
            } },
        },
    },
    content = {
        entities = { [ENTITY_ID] = { interactions = interaction_document } },
    },
}

local route = Building.resolve_field_location_route({
    scene_id = 0,
    building_interaction_mode = false,
    event_code = "event_7",
    event_index = 7,
    event_table_available = true,
    event_table = { 0, 0, 0, 0, 0, 0, 0, 21 },
    package_run_status = E.PackageRunStatus.NONE,
})
assert(route.kind == "area_event" and route.area_event_target == 21,
    "the imported event-table target remains documented")
assert(route.field_exit_prompt ~= nil,
    "scene 0/event 7 must dispatch the Roscoe handler instead of entering scene 21")
