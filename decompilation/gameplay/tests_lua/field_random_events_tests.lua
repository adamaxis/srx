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

local function run_haggard(choice, event_id, configuration)
    configuration = configuration or {}
    local dialogue_pages = {}
    local exit_calls = {}
    local random_calls = {}
    local prompt_titles = {}
    local prompt_speakers = {}
    local random_values = configuration.random_values or {
        [6] = { 0, 0 },
        [100] = { 0 },
    }
    local random_indices = {}
    local funds = 125
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = event_id or "haggard_man_collapse",
        },
        modules = {
            combat = Combat,
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
        party = function()
            return {
                [0] = {
                    attributes = {
                        body = configuration.body or 1,
                        intelligence = configuration.intelligence or 0,
                    },
                },
            }
        end,
        nuyen = function() return funds end,
        set_nuyen = function(value) funds = value end,
        ui = {
            say_text = function(text, continuation, speaker_id, text_profile)
                dialogue_pages[#dialogue_pages + 1] = {
                    text = text,
                    continuation = continuation,
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
            end,
            prompt_text = function(menu_id, prompt, options,
                                   speaker_id, text_profile)
                eq(menu_id, "haggard_man_action", "doctor menu id")
                prompt_titles[#prompt_titles + 1] = prompt
                prompt_speakers[#prompt_speakers + 1] = {
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
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
                    local bound = type(action) == "string" and
                        tonumber(action:match("^bounded_(%d+)$")) or nil
                    if bound == nil then
                        return "invalid_random_result"
                    end
                    random_indices[bound] = (random_indices[bound] or 0) + 1
                    local values = random_values[bound]
                    local value = type(values) == "table" and
                        values[random_indices[bound]] or nil
                    if value == nil then return "invalid_random_result" end
                    return "roll_" .. tostring(value)
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
    return task_result, dialogue_pages, exit_calls, funds, random_calls,
        prompt_titles, prompt_speakers
end

local haggard_result, haggard_pages, haggard_exit, haggard_funds,
      haggard_random, haggard_titles, haggard_prompt_speakers =
    run_haggard("help", nil, {
        intelligence = 0,
        random_values = { [6] = { 0 }, [100] = { 0 } },
    })
eq(haggard_result, "resolved", "doctor Help branch resolves")
eq(haggard_pages[1].text, "doctor-opening", "doctor source opening")
eq(haggard_pages[1].continuation, true,
   "doctor opening waits for its source continuation")
eq(haggard_pages[1].speaker_id, "haggard_man_event",
   "doctor opening uses its scene illustration speaker")
eq(haggard_pages[1].text_profile, "field.world_event",
   "doctor opening remains in the field event text profile")
eq(haggard_pages[2].text,
   "<You grab the man and hold him while the henchmen straight-jacket " ..
   "him and drag him off.>||Here's fifty nuyen. Now get lost.",
   "ROM 0x121188 Help result text")
eq(haggard_pages[2].continuation, false,
   "doctor Help result is the terminal page")
eq(haggard_funds, 175, "ROM 0x23CF2 Help reward adds 50 nuyen")
eq(table.concat(haggard_exit, ","), "stay,fade_83,lock_60",
   "measured doctor exit requests stay, its 83-frame black interval, and lock 60")
eq(haggard_titles[1], "<The man looks like a typical doctor to you.>",
   "ROM 0x23BB4 zero-success alternate introduction")
eq(haggard_prompt_speakers[1].speaker_id, "haggard_man_event",
   "doctor choices keep the scene illustration")
eq(haggard_prompt_speakers[1].text_profile, "field.world_event",
   "doctor choices keep the field event profile")
eq(haggard_pages[2].speaker_id, "haggard_man_event",
   "doctor result keeps the scene illustration")
eq(haggard_pages[2].text_profile, "field.world_event",
   "doctor result keeps the field event profile")
eq(table.concat(haggard_random, ","), "bounded_6,bounded_100",
   "ROM 0x23BB4 checks first-runner Intelligence before prompt selection")
local alternate_help_result, alternate_help_pages, _, alternate_help_funds,
      alternate_help_random, alternate_help_titles = run_haggard("help", nil, {
    intelligence = 1,
    random_values = { [6] = { 5, 0 }, [100] = { 50 } },
})
eq(alternate_help_result, "resolved",
   "ROM 0x12111C Help command 0xAC resolves")
eq(alternate_help_funds, 175,
   "ROM 0x23CE0 alternate Help command adds 50 nuyen")
eq(alternate_help_pages[2].text,
   "<They explain that they work for an insane asylum and that this is one " ..
   "of their patients.||After showing the proper credentials they give you " ..
   "50$ for your help, take their patient, and leave.>",
   "ROM 0x121178 alternate Help result text")
eq(alternate_help_titles[1],
   "<You're not sure what to make of this strange group.>",
   "ROM 0x23BB4 uses one-success Intelligence response text")
eq(table.concat(alternate_help_random, ","), "bounded_6,bounded_6,bounded_100",
   "ROM 0x23BB4 rolls the first runner's Intelligence before the prompt roll")

local standard_threaten_result, standard_threaten_pages, _,
      standard_threaten_funds, standard_threaten_random =
    run_haggard("threaten", nil, {
        intelligence = 1,
        random_values = { [6] = { 5, 0 }, [100] = { 50, 49 } },
    })
eq(standard_threaten_result, "resolved",
   "ROM 0x12111C Threaten dispatches command 0xA9")
eq(standard_threaten_funds, 175,
   "ROM 0x23CE0 low roll adds 50 nuyen on the standard prompt")
eq(standard_threaten_pages[2].text,
   "<They explain that they work for an insane asylum and that this is one " ..
   "of their patients.||After showing the proper credentials they give you " ..
   "50$ for your help, take their patient, and leave.>",
   "ROM 0x121178 standard Threaten result text")
eq(table.concat(standard_threaten_random, ","),
   "bounded_6,bounded_6,bounded_100,bounded_100",
   "standard Threaten performs the prompt roll before its outcome roll")

local standard_attack_result, _, standard_attack_exit,
      standard_attack_funds = run_haggard("attack", nil, {
    intelligence = 0,
    random_values = { [6] = { 0 }, [100] = { 50 } },
})
eq(type(standard_attack_result), "table",
   "ROM 0x12111C Attack return remains an explicit boundary")
eq(standard_attack_result.type, "source_boundary",
   "ROM 0x12111C Attack returns through 0x121180")
eq(#standard_attack_exit, 0,
   "standard Attack boundary does not complete the same-map exit")
eq(standard_attack_funds, 125,
   "standard Attack return pointer does not invent a reward")

for _, roll in ipairs({ 0, 49 }) do
    local threaten_result, threaten_pages, threaten_exit, threaten_funds,
          threaten_random = run_haggard("threaten", nil, {
        intelligence = 0,
        random_values = { [6] = { 0 }, [100] = { 0, roll } },
    })
    eq(threaten_result, "resolved",
       "ROM 0x23CB4 Threaten roll below 50 resolves")
    eq(threaten_funds, 225,
       "ROM 0x23CCE adds 100 nuyen for Threaten roll below 50")
    eq(threaten_pages[2].text:find("about dry", 1, true) ~= nil, true,
       "ROM 0x1211A0 Threaten success result text")
    eq(threaten_pages[2].continuation, false,
       "Threaten success result is terminal")
    eq(table.concat(threaten_random, ","),
       "bounded_6,bounded_100,bounded_100",
       "Threaten follows the Intelligence and prompt rolls with its 0-99 roll")
    eq(table.concat(threaten_exit, ","), "stay,fade_83,lock_60",
       "Threaten success uses the source same-map exit")
end

local patrol_result, patrol_pages, patrol_exit, patrol_funds,
      patrol_random = run_haggard("threaten", nil, {
    intelligence = 0,
    random_values = { [6] = { 0 }, [100] = { 0, 50 } },
})
eq(type(patrol_result), "table", "patrol branch is an explicit boundary")
eq(patrol_result.type, "source_boundary",
   "ROM 0x23CB4 roll 50 enters the patrol boundary")
eq(#patrol_pages, 1,
   "patrol boundary does not invent an unconverted police prompt")
eq(#patrol_exit, 0, "patrol boundary does not apply the help exit")
eq(patrol_funds, 125, "patrol branch does not award threat money")
eq(table.concat(patrol_random, ","),
   "bounded_6,bounded_100,bounded_100",
   "patrol branch rolls after the Intelligence and prompt checks")

local invalid_roll_result, _, _, invalid_roll_funds =
    run_haggard("threaten", nil, {
        intelligence = 0,
        random_values = { [6] = { 0 }, [100] = { 0, 100 } },
    })
eq(type(invalid_roll_result), "table",
   "invalid random result is an explicit source boundary")
eq(invalid_roll_result.type, "source_boundary",
   "out-of-range random service value remains an explicit boundary")
eq(invalid_roll_funds, 125,
   "invalid random result cannot award nuyen")

local standard_patrol_result, _, _, standard_patrol_funds =
    run_haggard("threaten", nil, {
        intelligence = 0,
        random_values = { [6] = { 0 }, [100] = { 50, 50 } },
    })
eq(type(standard_patrol_result), "table",
   "standard patrol branch remains an explicit boundary")
eq(standard_patrol_result.type, "source_boundary",
   "ROM 0x23C38 roll 50 enters the police prompt boundary")
eq(standard_patrol_funds, 125,
   "standard patrol branch does not award the 50 nuyen payout")

local attack_reward_result, attack_reward_pages, attack_reward_exit,
      attack_reward_funds, attack_reward_random = run_haggard("attack", nil, {
    intelligence = 0,
    body = 2,
    random_values = { [6] = { 0, 5, 0, 5, 0 }, [100] = { 0 } },
})
eq(attack_reward_result, "resolved",
   "ROM 0x23C52 Attack with more than one Body success resolves")
eq(attack_reward_funds, 225,
   "ROM 0x23C74 adds 100 nuyen after more than one Body success")
eq(attack_reward_pages[2].text:find("about dry", 1, true) ~= nil, true,
   "ROM 0x1211A0 Attack success page")
eq(table.concat(attack_reward_random, ","),
   "bounded_6,bounded_100,bounded_6,bounded_6,bounded_6,bounded_6",
   "Attack uses first-runner Intelligence and Body dice pools in source order")
eq(table.concat(attack_reward_exit, ","), "stay,fade_83,lock_60",
   "no-combat Attack result returns through the same-map exit")

local unsupported_result, _, unsupported_exit, unsupported_funds =
    run_haggard("attack", nil, {
        intelligence = 0,
        body = 1,
        random_values = { [6] = { 0, 0 }, [100] = { 0 } },
    })
eq(type(unsupported_result), "table",
   "unimplemented doctor combat branch stays an explicit boundary")
eq(unsupported_result.type, "source_boundary",
   "unimplemented doctor combat branch does not become a terminal exit")
eq(#unsupported_exit, 0,
   "source boundary applies no descriptor exit effects")
eq(unsupported_funds, 125,
   "source boundary does not award the Help reward")

local one_success_attack, _, _, one_success_funds = run_haggard(
    "attack", nil, {
        intelligence = 0,
        body = 1,
        random_values = { [6] = { 0, 5, 0 }, [100] = { 0 } },
    })
eq(type(one_success_attack), "table",
   "one-success Attack keeps its source combat list boundary")
eq(one_success_attack.type, "source_boundary",
   "ROM 0x23C52 one success queues its bodyguard list")
eq(one_success_funds, 125,
   "one-success Attack cannot take the no-combat payout")

game.location.event_id = "different_event"
eq(RandomEvents.present_haggard_man(), nil,
   "unrelated event cannot enter the escaped-patient handler")
eq(Combat.roll_thresholded_successes(-1, 4, function() return 0 end), nil,
   "negative dice pool rejected")

print(("field_random_events_tests: %d checks"):format(checks))
