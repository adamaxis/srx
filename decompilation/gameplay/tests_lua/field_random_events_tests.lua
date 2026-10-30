package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local Combat = require("combat")
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
eq(Combat.roll_thresholded_successes(-1, 4, function() return 0 end), nil,
   "negative dice pool rejected")

print(("field_random_events_tests: %d checks"):format(checks))
