package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local E = require("enums")
local Building = require("route_test_support").wrap(require("building"))
local Combat = require("combat")
local FIELD_ROUTE_FLAG_BIT = E.FieldRouteFlagBit or {
    SCENE_20_ENTRY = 26,
    HOLLYWOOD_FALSE_NAME = 27,
    HOLLYWOOD_CLEARANCE_BADGE = 28,
}

local LOCATION_ID = "hollywood_correctional_facility"
local function read_json(path)
    local file = assert(io.open(path, "rb"), "cannot read " .. path)
    local decoded = Json.decode(file:read("*a"))
    file:close()
    return decoded
end

local interaction_document = read_json(
    "decompilation/gameplay/world/entities/" .. LOCATION_ID ..
    "/interactions.json")
local text_catalog = read_json("decompilation/assets/dialogue/script-text.json")
local speaker_catalog = read_json("decompilation/gameplay/dialogue/speakers.json")
local menu_catalog = read_json("decompilation/gameplay/dialogue/menus.json")

local failures, total = {}, 0
local function test(name, callback)
    total = total + 1
    local ok, failure = pcall(callback)
    if not ok then
        failures[#failures + 1] = name .. ": " .. tostring(failure)
    end
end

local function expect(value, message)
    assert(value, message)
end

local function flag_fact(bit)
    return "field_route_flag_bit_" .. bit
end

local function fresh_game(options)
    options = options or {}
    local state = {}
    for bit, value in pairs(options.flags or {}) do
        state[flag_fact(bit)] = value
    end
    local capture = { events = {}, prompts = {}, choices = {} }
    local random = options.random_values or {}
    local random_index = 0
    game = {
        location = { type = "building", id = LOCATION_ID },
        field = {
            package_run_status = options.package_status or
                E.PackageRunStatus.NONE,
        },
        state = state,
        modules = { combat = Combat },
        party = function() return options.party or {} end,
        active_slot = function() return 0 end,
        services = {
            call = function(domain, action)
                expect(domain == "random" and action == "bounded_6",
                    "Hollywood gate requested an unexpected service")
                random_index = random_index + 1
                capture.random_calls = random_index
                return "roll_" .. (random[random_index] or 0)
            end,
        },
        ui = {
            say = function(event_id, speaker_id, text_profile)
                capture.events[#capture.events + 1] = {
                    event_id = event_id,
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
                return true
            end,
            prompt_text_key = function(menu_id, text_key, choices,
                                       speaker_id, text_profile)
                capture.prompts[#capture.prompts + 1] = {
                    menu_id = menu_id,
                    text_key = text_key,
                    choices = choices,
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
                return table.remove(capture.choices, 1)
            end,
        },
    }
    for _, choice in ipairs(options.choices or {}) do
        capture.choices[#capture.choices + 1] = choice
    end
    return state, capture
end

local function route_for_gate()
    local entity = { id = LOCATION_ID, links = {} }
    game.modules.world_content_data = {
        entities = { [LOCATION_ID] = entity },
        areas = { [2] = { { instance_id = "gate", entity_id = LOCATION_ID } } },
    }
    game.content = {
        entities = { [LOCATION_ID] = { interactions = interaction_document } },
        location_conditions = { schemaVersion = 1, conditions = {} },
    }
    return Building.resolve_field_location_route({
        scene_id = 2,
        event_code = "event_0",
        event_index = 0,
        event_table_available = true,
        event_table = { 20 },
        transition = { kind = "enter_scene", destination_scene_id = 20 },
        building_interaction_mode = false,
        package_run_status = game.field.package_run_status,
    })
end

local function expect_gate()
    local ok, gate = pcall(require, "hollywood_correctional_gate")
    expect(ok and type(gate) == "table",
        "scene-2/event-0 has no Hollywood guard-gate handler")
    return gate
end

local attack_runner = {
    attributes = { quickness = 9, intelligence = 9, willpower = 9 },
    skills = { firearms = 8, pistols = 8, smgs = 8, shotguns = 8 },
    equipped_action_code = E.ItemId.AMERICAN_L36,
    posture = E.Posture.FULL_OFFENSE,
    equipment = {},
}

local function success_roll_values(success_count)
    local values = {}
    for _ = 1, success_count do
        values[#values + 1] = 5
        values[#values + 1] = 0
    end
    for _ = 1, 32 do values[#values + 1] = 0 end
    return values
end

test("route requires the entry dialogue while retaining Ilene service 19", function()
    fresh_game({})
    local route = route_for_gate()
    expect(route.kind == "area_event" and route.area_event_target == 20,
        "ROM event-table destination must stay scene 20")
    expect(route.entry_dialogue_module == "hollywood_correctional_gate" and
           route.entry_dialogue_function == "present_entry" and
           route.entry_continue_result_id == "enter_scene_20",
        "scene 2/event 0 must gate scene 20 behind the dialogue result")
    expect(route.route_preparation.module == "ilene" and
           route.route_preparation.function_name == "prepare_scene_20_entry",
        "ROM service 19 must remain earlier in the route chain")
end)

test("Hollywood access bits use the named ROM positions", function()
    expect(E.FieldRouteFlagBit.SCENE_20_ENTRY == 26 and
           E.FieldRouteFlagBit.HOLLYWOOD_FALSE_NAME == 27 and
           E.FieldRouteFlagBit.HOLLYWOOD_CLEARANCE_BADGE == 28,
        "Hollywood route flags must name the FFFC2C source bits")
    expect(E.HollywoodGateAttack.ROLL_DIFFICULTY == 6 and
           E.HollywoodGateAttack.MINIMUM_ENTRY_SUCCESSES == 3 and
           E.HollywoodGateAttack.EASY_AMBUSH_SUCCESSES == 5,
        "Hollywood attack roll thresholds must match cmd 0x65")
end)

test("no access flags shows entrance and guard choices, then leave stays", function()
    local state, capture = fresh_game({ choices = { "leave" } })
    local gate = expect_gate()
    expect(gate.present_entry() == "left",
        "leave choice must suppress the event-table transition")
    expect(capture.events[1].event_id == "hollywood_gate_entrance" and
           capture.prompts[1].menu_id == "hollywood_gate_guard_choices" and
           capture.prompts[1].text_key == "hollywood.gate.guard.opening" and
           capture.events[2].event_id == "hollywood_gate_farewell",
        "no-flag path must show page 0x1113D2, choices 0x1113E6, and leave page 0x11144A")
    expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == nil and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == nil and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == nil,
        "no-flag leave path must not change FFFC2C access bits")
end)

test("badge bit has priority, clears only itself, and opens scene 20", function()
    local state, capture = fresh_game({ flags = {
        [FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY] = true,
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = true,
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = true,
    } })
    local gate = expect_gate()
    expect(gate.present_entry() == "enter_scene_20",
        "badge access must continue to scene 20")
    expect(capture.events[2].event_id == "hollywood_gate_sarge_pass",
        "bit 28 path must show the Sarge badge page")
    expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "bit 28 access must clear only bit 28")
end)

test("false-name bit clears only itself and opens scene 20", function()
    local state, capture = fresh_game({ flags = {
        [FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY] = true,
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = true,
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = false,
    } })
    local gate = expect_gate()
    expect(gate.present_entry() == "enter_scene_20",
        "false-name access must continue to scene 20")
    expect(capture.events[2].event_id == "hollywood_gate_false_name_pass",
        "bit 27 path must show the false-name page")
    expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == false and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "bit 27 access must clear only bit 27")
end)

test("both access bits consume only badge bit 28", function()
    local state, capture = fresh_game({ flags = {
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = true,
        [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = true,
    } })
    local gate = expect_gate()
    expect(gate.present_entry() == "enter_scene_20" and
           capture.events[2].event_id == "hollywood_gate_sarge_pass",
        "both flags must take bit 28's higher-priority page")
    expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "bit 27 must remain set when bit 28 is consumed")
end)

test("story choice advances from 0x11141A to the repeated choices at 0x11142E", function()
    local _, capture = fresh_game({ choices = { "story", "leave" } })
    local gate = expect_gate()
    expect(gate.present_entry() == "left",
        "story followed by leave must remain in scene 2")
    expect(capture.prompts[1].menu_id == "hollywood_gate_guard_choices" and
           capture.events[2].event_id == "hollywood_gate_story_first" and
           capture.prompts[2].menu_id == "hollywood_gate_story_choices" and
           capture.prompts[2].text_key == "hollywood.gate.story.second" and
           capture.prompts[2].choices[1] == "attack" and
           capture.prompts[2].choices[2] == "story" and
           capture.prompts[2].choices[3] == "leave",
        "story route must show page 0x11141A, then choices at 0x11142E")
end)

test("cmd 0x65 with 3-4 successes enters and sets only bit 26", function()
    local state, capture = fresh_game({
        choices = { "attack" },
        flags = {
            [FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY] = false,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = false,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = false,
        },
        party = { [0] = attack_runner },
        random_values = success_roll_values(3),
    })
    local gate = expect_gate()
    expect(gate.present_entry() == "enter_scene_20",
        "three successes must follow the 3-4 success branch")
    expect(capture.events[#capture.events].event_id == "hollywood_gate_attack_success" and
           capture.random_calls == 19 and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == false and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "3-4 success must set bit 26 only and preserve bits 27/28")
end)

test("cmd 0x65 with at least 5 successes shows ambush page and opens", function()
    local state, capture = fresh_game({
        choices = { "attack" },
        flags = {
            [FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY] = false,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = false,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = false,
        },
        party = { [0] = attack_runner },
        random_values = success_roll_values(5),
    })
    local gate = expect_gate()
    expect(gate.present_entry() == "enter_scene_20",
        "five successes must follow the ambush branch")
    expect(capture.events[#capture.events].event_id == "hollywood_gate_attack_ambush" and
           capture.random_calls == 21 and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == false and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "5+ success must set bit 26 only and preserve bits 27/28")
end)

test("cmd 0x65 miss displays failure and cancels entry", function()
    local state, capture = fresh_game({
        choices = { "attack" },
        flags = {
            [FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY] = true,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = false,
            [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = false,
        },
        party = { [0] = attack_runner },
        random_values = success_roll_values(0),
    })
    local gate = expect_gate()
    expect(gate.present_entry() == "left" and
           capture.events[#capture.events].event_id == "hollywood_gate_attack_missed",
        "under-3 path must show page 0x111402 and remain in scene 2")
    expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.SCENE_20_ENTRY)] == true and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == false and
           state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == false,
        "miss path must leave all FFFC2C access bits unchanged")
end)

test("package pages stay text-only and do not consume access flags", function()
    for _, scenario in ipairs({
        { status = E.PackageRunStatus.PAYMENT_DUE,
          event = "hollywood_gate_package_payment" },
        { status = E.PackageRunStatus.HANDOFF_DUE,
          event = "hollywood_gate_package_handoff" },
    }) do
        local state, capture = fresh_game({
            package_status = scenario.status,
            flags = {
                [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME] = true,
                [FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE] = true,
            },
        })
        local gate = expect_gate()
        expect(gate.present_entry() == "left" and
               capture.events[2].event_id == scenario.event,
            "package status must show its matching Hollywood page only")
        expect(state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_FALSE_NAME)] == true and
               state[flag_fact(FIELD_ROUTE_FLAG_BIT.HOLLYWOOD_CLEARANCE_BADGE)] == true,
            "package page must leave access flags unchanged")
    end
end)

test("service-19 Ilene preparation remains active before the new gate", function()
    fresh_game({})
    local route = route_for_gate()
    expect(route.route_preparation.module == "ilene" and
           route.route_preparation.function_name == "prepare_scene_20_entry",
        "scene-2/event-0 must keep the Ilene service-19 task")
    game.state[E.StoryFact.RED_BUFFALO_WOMAN] = true
    game.state[E.StoryFact.ILENE_MET] = false
    game.state.field_mode_effect_flag_bit_0 = true
    game.state.notebook_tip_flag_bit_76 = false
    local Ilene = require("ilene")
    expect(Ilene.prepare_scene_20_entry() == "continue" and
           game.state.pending_world_effect == true,
        "Ilene's existing pending actor preparation must still run unchanged")
end)

test("dialogue texts and choice labels match the ROM-backed catalog", function()
    expect(text_catalog.texts["hollywood.gate.entrance"] ==
        "<This is the entrance to the Hollywood Correctional Facility, home to a large number of criminals and scum of the city.>",
        "entrance page text must match ROM address 0x1161E6")
    expect(text_catalog.texts["hollywood.gate.choice.attack"] ==
        "<Open fire on the Guard Post.>" and
        text_catalog.texts["hollywood.gate.choice.story"] ==
        "What's the story with this place?" and
        text_catalog.texts["hollywood.gate.choice.leave"] ==
        "I'll talk to you later, chum.",
        "three choice labels must match ROM addresses 0x104CF4/0x104D13/0x104FEB")
    local expected_texts = {
        ["hollywood.gate.package_handoff"] =
            "<A guard walks up to you.>||You the runner sent from Johnson?||<When you say yes, he hands you a package and walks away.>",
        ["hollywood.gate.package_payment"] =
            "<A guard approaches and takes the package.||He checks it out and gives you your nuyen when satisfied.>",
        ["hollywood.gate.sarge_pass"] =
            "<As you approach the gate, Sarge appears and speaks very loudly, so as to be easily overheard.>||Oh, uh, look, official visitors.  Here is your offical pass.||<His hands shake as he pins a clearance badge on you.  You move on, much to his relief.>",
        ["hollywood.gate.false_name_pass"] =
            "<You walk to the security gate and speak to the guard.||He checks the false name you give him against a list in the computer and clears you for entry.||The gate swings open.>",
        ["hollywood.gate.story.first"] =
            "You must be tourists!||Hollywood Correctional is an underground prison.||We have patrols around the clock and haven't had a breakout since, oh, 2028.||That's year, not hour, vid-clown.",
        ["hollywood.gate.story.second"] =
            "Since then, we have increased the security to keep the scumbags away from law-abiding citizens.||<He frowns at you after his last remark.>",
        ["hollywood.gate.farewell"] = "Good day, citizen.",
        ["hollywood.gate.attack_missed"] =
            "<You open fire on the guards.||Your shots miss the mark and soon the guards are returning fire.>",
        ["hollywood.gate.attack_success"] =
            "<You kill the first guard and he slumps over the controls that open the gate.||You dash into the prison with the other guards hot on your trail.>",
        ["hollywood.gate.attack_ambush"] =
            "<You easily ambush the security guards.||You sprint into the prison.>",
    }
    for text_key, expected in pairs(expected_texts) do
        expect(text_catalog.texts[text_key] == expected,
            "dialogue text must match its ROM-backed source string: " .. text_key)
    end
    expect(speaker_catalog.profiles.field_narrator ~= nil and
           menu_catalog.profiles["field.world_event"] ~= nil and
           menu_catalog.profiles["field.world_event"].hollywood_gate_guard_choices ~= nil and
           menu_catalog.profiles["field.world_event"].hollywood_gate_story_choices ~= nil,
        "Hollywood dialogue must use the existing field narrator and menu profile")
end)

if #failures > 0 then
    for _, failure in ipairs(failures) do print("FAIL " .. failure) end
    print(string.format("hollywood gate tests failed (%d/%d)",
                        #failures, total))
    os.exit(1)
end

print(string.format("hollywood gate tests passed (%d)", total))
