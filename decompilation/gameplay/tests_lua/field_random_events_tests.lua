package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local Combat = require("combat")
local DialogueFramework = require("dialogue_framework")
local E = require("enums")
local RandomEvents = require("field_random_events")

local checks = 0
local function eq(actual, expected, label)
    assert(actual == expected,
        ("%s: expected %s, got %s"):format(label, tostring(expected),
                                             tostring(actual)))
    checks = checks + 1
end

local function member(active, quickness)
    return {
        runner_facts = { lifecycle = { active = active } },
        attributes = { quickness = quickness },
    }
end

local function run_event(party, rolls)
    local pages = {}
    local alarm_calls = 0
    local random_calls = 0
    local result = nil
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = "hidden_camera",
        },
        modules = {
            combat = Combat,
            world_content_data = {
                world_activity_event_content = {
                    { event_id = "hidden_camera", opening_text = "opening" },
                },
                world_activity_event_choices = {
                    { event_id = "hidden_camera", choice_order = 1,
                      choice_id = "continue" },
                },
            },
        },
        party = function() return party end,
        ui = {
            say_text = function(text, continuation)
                pages[#pages + 1] = {
                    text = text,
                    continuation = continuation,
                }
            end,
        },
        services = {
            call = function(domain, action)
                if domain == "random" then
                    eq(action, "bounded_6", "source die request")
                    random_calls = random_calls + 1
                    local roll = rolls[random_calls]
                    return roll ~= nil and ("roll_" .. roll) or nil
                end
                if domain == "field" and action == "raise_alarm" then
                    alarm_calls = alarm_calls + 1
                    return "applied"
                end
                return nil
            end,
        },
    }
    result = RandomEvents.present_hidden_camera()
    return result, pages, alarm_calls, random_calls
end

local result, pages, alarms, random_calls = run_event(
    { [0] = member(true, 0), [1] = member(false, 0),
      [2] = member(false, 0) }, { 5, 0 })
eq(result, "resolved", "one-runner event result")
eq(#pages, 2, "one-runner page count")
eq(pages[1].text, "opening", "source opening text")
eq(pages[1].continuation, true, "single MORE continuation")
eq(pages[2].text:find("Nothing happens", 1, true) ~= nil, true,
   "success result text")
eq(pages[2].continuation, false, "success result has no continuation")
eq(alarms, 0, "success does not raise alarm")
eq(random_calls, 2, "zero-die roll still rolls once and six explodes")

result, pages, alarms, random_calls = run_event(
    { [0] = member(true, 2), [1] = member(true, 3),
      [2] = member(false, 0) }, { 3, 0 })
eq(result, "resolved", "two-runner event result")
eq(alarms, 1, "two-runner failure raises alarm")
eq(random_calls, 2, "two-runner average quickness gives two dice")
eq(pages[2].text:find("alarm goes off", 1, true) ~= nil, true,
   "alarm result text")

result, pages, alarms, random_calls = run_event(
    { [0] = member(true, 1), [1] = member(true, 1),
      [2] = member(true, 1) }, { 4 })
eq(result, "resolved", "three-runner event result")
eq(alarms, 1, "three-runner target rises to six")
eq(random_calls, 1, "three-runner average quickness gives one die")

game.location.event_id = "different_event"
eq(RandomEvents.present_hidden_camera(), nil,
   "unrelated event cannot enter hidden-camera handler")

local function run_haggard(choice, event_id, random_roll)
    local dialogue_pages = {}
    local exit_calls = {}
    local random_calls = {}
    local funds = 125
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = event_id or "haggard_man_collapse",
        },
        modules = {
            world_content_data = {
                world_activity_event_content = {
                    { event_id = "haggard_man_collapse",
                      opening_text = "doctor-opening" },
                },
                world_activity_event_choices = {
                    { event_id = "haggard_man_collapse", choice_order = 1,
                      choice_id = "continue" },
                },
            },
        },
        nuyen = function() return funds end,
        set_nuyen = function(value) funds = value end,
        ui = {
            say_text = function(text, continuation)
                dialogue_pages[#dialogue_pages + 1] = {
                    text = text,
                    continuation = continuation,
                }
            end,
            prompt_text = function(menu_id, prompt, options)
                eq(menu_id, "haggard_man_action", "doctor menu id")
                eq(prompt, "", "doctor menu prompt")
                eq(#options, 3, "doctor has the three source choices")
                eq(options[1].id, "attack", "first source choice")
                eq(options[2].id, "threaten", "second source choice")
                eq(options[3].id, "help", "Help is the third source choice")
                return choice
            end,
        },
        services = {
            call = function(domain, action)
                if domain == "random" then
                    random_calls[#random_calls + 1] = action
                    if action ~= "bounded_100" or random_roll == nil then
                        return "invalid_random_result"
                    end
                    return "roll_" .. tostring(random_roll)
                end
                eq(domain, "dialogue_exit",
                   "exit effects use the common executor service")
                exit_calls[#exit_calls + 1] = action
                return "applied"
            end,
        },
    }
    local task_result = DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "field_random_events", "present_haggard_man")
    return task_result, dialogue_pages, exit_calls, funds, random_calls
end

local haggard_result, haggard_pages, haggard_exit, haggard_funds =
    run_haggard("help")
eq(haggard_result, "resolved", "doctor Help branch resolves")
eq(haggard_pages[1].text, "doctor-opening", "doctor source opening")
eq(haggard_pages[1].continuation, true,
   "doctor opening waits for its source continuation")
eq(haggard_pages[2].text:find("restrained", 1, true) ~= nil, true,
   "Help result summarizes the restrained-patient outcome")
eq(haggard_pages[2].continuation, false,
   "doctor Help result is the terminal page")
eq(haggard_funds, 175, "ROM 0x23CF2 Help reward adds 50 nuyen")
eq(table.concat(haggard_exit, ","), "stay,fade_83,lock_60",
   "measured doctor exit requests stay, its 83-frame black interval, and lock 60")

for _, roll in ipairs({ 0, 49 }) do
    local threaten_result, threaten_pages, threaten_exit, threaten_funds,
          threaten_random = run_haggard("threaten", nil, roll)
    eq(threaten_result, "resolved",
       "ROM 0x23CB4 Threaten roll below 50 resolves")
    eq(threaten_funds, 225,
       "ROM 0x23CCE adds 100 nuyen for Threaten roll below 50")
    eq(threaten_pages[2].text:find("about dry", 1, true) ~= nil, true,
       "ROM 0x1211A0 Threaten success result text")
    eq(threaten_pages[2].continuation, false,
       "Threaten success result is terminal")
    eq(table.concat(threaten_random, ","), "bounded_100",
       "Threaten uses the ROM 0-99 random range")
    eq(table.concat(threaten_exit, ","), "stay,fade_83,lock_60",
       "Threaten success uses the source same-map exit")
end

local patrol_result, patrol_pages, patrol_exit, patrol_funds,
      patrol_random = run_haggard("threaten", nil, 50)
eq(patrol_result.type, "source_boundary",
   "ROM 0x23CB4 roll 50 enters the patrol boundary")
eq(#patrol_pages, 1,
   "patrol boundary does not invent an unconverted police prompt")
eq(#patrol_exit, 0, "patrol boundary does not apply the help exit")
eq(patrol_funds, 125, "patrol branch does not award threat money")
eq(table.concat(patrol_random, ","), "bounded_100",
   "patrol branch consumes exactly one source roll")

local invalid_roll_result, _, _, invalid_roll_funds =
    run_haggard("threaten", nil, 100)
eq(invalid_roll_result.type, "source_boundary",
   "out-of-range random service value remains an explicit boundary")
eq(invalid_roll_funds, 125,
   "invalid random result cannot award nuyen")

local unsupported_result, _, unsupported_exit, unsupported_funds =
    run_haggard("attack")
eq(type(unsupported_result), "table",
   "unimplemented doctor combat branch stays an explicit boundary")
eq(unsupported_result.type, "source_boundary",
   "unimplemented doctor combat branch does not become a terminal exit")
eq(#unsupported_exit, 0,
   "source boundary applies no descriptor exit effects")
eq(unsupported_funds, 125,
   "source boundary does not award the Help reward")

game.location.event_id = "different_event"
eq(RandomEvents.present_haggard_man(), nil,
   "unrelated event cannot enter the escaped-patient handler")
eq(Combat.roll_thresholded_successes(-1, 4, function() return 0 end), nil,
   "negative dice pool rejected")

print(("field_random_events_tests: %d checks"):format(checks))
