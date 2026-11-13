package.path = "decompilation/gameplay/scripts/?.lua;" .. package.path

local Combat = require("combat")
local DialogueFramework = require("dialogue_framework")
local E = require("enums")
local Encounters = require("field_encounters")
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

local function run_faction_disturbance(
    mafia_route, halloweener_route, queue_result)
    local pages = {}
    local route_calls = {}
    local sequence_calls = {}
    local queue_calls = {}
    local flags = {
        faction_disturbance_mafia_route_enabled = mafia_route,
        faction_disturbance_halloweener_route_enabled = halloweener_route,
    }
    local sequence_members = {}
    for _, route in ipairs({
        { id = E.WorldEffectSequence.FACTION_MAFIA_EYE_FIVERS,
          profile = "mafia_eye5r_k07_item0c" },
        { id = E.WorldEffectSequence.FACTION_HALLOWEENERS,
          profile = "weenr2_k08_item01" },
    }) do
        for order = 0, 3 do
            sequence_members[#sequence_members + 1] = {
                scene_id = 0,
                sequence_id = route.id,
                sequence_order = order,
                profile_id = route.profile,
                placement_policy =
                    E.WorldEffectPlacementPolicy.MAP_DERIVED_CANDIDATES,
            }
        end
    end
    local dynamic_actor_profiles = {
        {
            profile_id = "mafia_eye5r_k07_item0c",
            actor_name = "mafia eye5r",
            visual_profile_id = E.ActorVisualProfile.GHOUL_EYE_FIVER,
            sprite_stem = "actor-kind-7-palette-2",
            combat_body = 5,
        },
        {
            profile_id = "weenr2_k08_item01",
            actor_name = "weenr2",
            visual_profile_id = E.ActorVisualProfile.YAKUZA_WEENR,
            sprite_stem = "yakuza_weenr_k08_item0a",
            combat_body = 4,
        },
    }
    for _, profile in ipairs(dynamic_actor_profiles) do
        profile.scene_id = 0
        profile.sequence_order = 0
        profile.sequence_id = "profile_catalog"
        profile.selection_kind = "weighted_encounter"
        profile.action_kind = "item"
        profile.action_id = "streetline_special"
        profile.actor_facts = { interaction = {} }
        profile.combat_cyberware = {}
        profile.combat_traits = {}
        profile.inventory = {}
        profile.weapon_action_policies = {}
        profile.physical_meter = 10
        profile.combat_mental_meter = 10
        profile.posture_reduction_percent = 0
        profile.combat_quickness = 4
        profile.combat_strength = 4
        profile.combat_intelligence = 4
        profile.combat_willpower = 4
        profile.combat_melee_combat = 1
        profile.combat_sorcery = 0
        profile.combat_firearms = 1
        profile.combat_pistols = 1
        profile.combat_smgs = 1
        profile.combat_shotguns = 1
        profile.equipped_armor_id = "none"
        profile.activity_probe_enabled = false
        profile.action_delay_modifier = 0
        profile.primary_action_profile_enabled = false
        profile.secondary_action_profile_enabled = false
        profile.force = 0
        profile.spell_trial_cap = 0
    end
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = "faction_disturbance",
        },
        modules = {
            world_content_data = {
                world_activity_event_content = {
                    { event_id = "faction_disturbance",
                      opening_text = "disturbance opening" },
                },
                world_activity_event_choices = {
                    { event_id = "faction_disturbance", choice_order = 1,
                      choice_id = "continue" },
                },
                dynamic_world_activity_actors = dynamic_actor_profiles,
                field_actor_sequence_members = sequence_members,
            },
        },
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
                if domain == "story" and flags[action] ~= nil then
                    route_calls[#route_calls + 1] = action
                    return flags[action]
                end
                if domain == "field" and action == "scene_reference" then
                    return "scene_0"
                end
                if domain == "story" and
                   action:match("^queue_world_effect_") then
                    queue_calls[#queue_calls + 1] = action
                    return queue_result or "queued"
                end
                return nil
            end,
        },
    }
    local resolve_sequence = Encounters.resolve_sequence
    Encounters.resolve_sequence = function(scene_id, sequence_id)
        sequence_calls[#sequence_calls + 1] = {
            scene_id = scene_id,
            sequence_id = sequence_id,
        }
        return resolve_sequence(scene_id, sequence_id)
    end
    local result = RandomEvents.present_faction_disturbance()
    Encounters.resolve_sequence = resolve_sequence
    return result, pages, route_calls,
           sequence_calls, queue_calls
end

local faction_result, faction_pages, faction_services =
    run_faction_disturbance("disabled", "disabled")
eq(faction_result, "resolved",
   "ROM 0x23876 neutral faction route resolves")
eq(#faction_pages, 2, "neutral faction route has opening and result pages")
eq(faction_pages[1].text, "disturbance opening",
   "neutral route uses the extracted event opening")
eq(faction_pages[1].continuation, true,
   "neutral route opening waits for its single MORE command")
-- ROM 0x23876 reaches the no-flag text at 0x11D128 without spawning actors.
eq(faction_pages[2].text,
   "<Impressive figures pass through the parting crowd.>||" ..
   "Your first impression is that they are veteran shadowrunners.||" ..
   "Then you see they are wearing gang colors!>",
   "neutral faction route uses the ROM result record")
eq(faction_pages[2].continuation, false,
   "neutral faction result is a terminal text page")
eq(table.concat(faction_services, ","),
   "faction_disturbance_mafia_route_enabled," ..
   "faction_disturbance_halloweener_route_enabled",
   "ROM bit-1 route check precedes the bit-2 route check")

local mafia_result, mafia_pages, mafia_services,
      mafia_sequence_calls, mafia_queue_calls =
    run_faction_disturbance("enabled", "disabled")
eq(mafia_result, "resolved", "ROM 0x23876 Mafia route resolves")
eq(#mafia_pages, 2, "Mafia route shows its attack result")
eq(mafia_pages[2].text,
   "<A gang of Eye-Fivers jump out of the crowd.>||" ..
   "So, you chose to serve the Yakuza?||" ..
   "Then you deserve to die like the other enemies of the Mafia!",
   "Mafia route uses the result record at ROM 0x11D098")
eq(mafia_pages[2].continuation, false,
   "Mafia result is a terminal page before the queued map sequence")
eq(table.concat(mafia_services, ","),
   "faction_disturbance_mafia_route_enabled",
   "ROM bit-1 priority skips the bit-2 route check")
eq(#mafia_sequence_calls, 1,
   "Mafia route resolves its source actor list once")
eq(mafia_sequence_calls[1].sequence_id,
   "faction_mafia_eye_fivers",
   "Mafia route uses a semantic actor-sequence ID")
eq(table.concat(mafia_queue_calls, ","),
   "queue_world_effect_0_4_faction_mafia_eye_fivers",
   "Mafia route queues four map-derived actors")

local halloween_result, halloween_pages, halloween_services,
      halloween_sequence_calls, halloween_queue_calls =
    run_faction_disturbance("disabled", "enabled")
eq(halloween_result, "resolved", "ROM 0x23876 Halloweener route resolves")
eq(#halloween_pages, 2, "Halloweener route shows its attack result")
eq(halloween_pages[2].text,
   "<Some Halloweeners appear out of the crowd.>||" ..
   "Hey, Mafia boy!  You chose the wrong side to ally with!||" ..
   "Yakuza rules this town!",
   "Halloweener route uses the result record at ROM 0x11D019")
eq(table.concat(halloween_services, ","),
   "faction_disturbance_mafia_route_enabled," ..
   "faction_disturbance_halloweener_route_enabled",
   "Halloweener route follows a clear ROM bit-1 check")
eq(halloween_sequence_calls[1].sequence_id,
   "faction_halloweeners",
   "Halloweener route resolves its semantic actor list")
eq(table.concat(halloween_queue_calls, ","),
   "queue_world_effect_0_4_faction_halloweeners",
   "Halloweener route queues four map-derived actors")

local both_routes_result, both_routes_pages, both_routes_services,
      both_routes_sequences =
    run_faction_disturbance("enabled", "enabled")
eq(both_routes_result, "resolved",
   "simultaneous faction bits follow the ROM priority branch")
eq(both_routes_pages[2].text, mafia_pages[2].text,
   "bit 1 selects the Mafia outcome when both bits are set")
eq(table.concat(both_routes_services, ","),
   "faction_disturbance_mafia_route_enabled",
   "both set bits skip the ROM bit-2 query")
eq(both_routes_sequences[1].sequence_id,
   "faction_mafia_eye_fivers",
   "both set bits queue only the Mafia actor sequence")

local busy_faction_result, busy_faction_pages =
    run_faction_disturbance("enabled", "disabled", "busy")
eq(busy_faction_result.type, "source_boundary",
   "a rejected actor-list queue remains an explicit boundary")
eq(#busy_faction_pages, 1,
   "a rejected actor-list queue does not claim the fight started")

local unknown_faction_result, unknown_faction_pages =
    run_faction_disturbance(nil, "disabled")
eq(unknown_faction_result.type, "source_boundary",
   "unknown faction flags do not assume the no-flag ROM branch")
eq(#unknown_faction_pages, 1,
   "unknown faction flags do not display an unverified result")

local function run_haggard(choice, event_id, configuration)
    configuration = configuration or {}
    local dialogue_pages = {}
    local exit_calls = {}
    local random_calls = {}
    local prompt_titles = {}
    local prompt_speakers = {}
    local queued_world_effects = {}
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
            scene_id = configuration.scene_id or 2,
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
                if domain == "field" and action == "scene_reference" then
                    return "scene_" .. tostring(game.location.scene_id)
                end
                if domain == "story" and
                   action:match("^queue_world_effect_") then
                    queued_world_effects[#queued_world_effects + 1] = action
                    return configuration.queue_result or "queued"
                end
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
        prompt_titles, prompt_speakers, queued_world_effects
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

local zero_success_attack, zero_success_pages, zero_success_exit,
      zero_success_funds, _, _, _, zero_success_effects =
    run_haggard("attack", nil, {
        intelligence = 0,
        body = 1,
        random_values = { [6] = { 0, 0 }, [100] = { 0 } },
    })
eq(zero_success_attack, "resolved",
   "ROM 0x23C52 zero-success Attack starts the source actor sequence")
eq(zero_success_pages[2].text,
   "Who do you think you're scaring?||Kill them all!!",
   "ROM 0x121190 zero-success Attack result text")
eq(zero_success_pages[2].speaker_id, "haggard_man_event",
   "zero-success Attack keeps the doctor illustration")
eq(zero_success_pages[2].text_profile, "field.world_event",
   "zero-success Attack keeps the field event text profile")
eq(zero_success_effects[1],
   "queue_world_effect_2_3_haggard_man_gator_shaman_and_bodyguards",
   "ROM 0x1D704C starts the three-actor map-derived sequence in Scene 2")
eq(#zero_success_effects, 1,
   "zero-success Attack queues exactly one actor sequence")
eq(zero_success_funds, 125,
   "zero-success Attack does not award the no-combat nuyen")
eq(table.concat(zero_success_exit, ","), "stay,fade_83,lock_60",
   "zero-success Attack returns through the common same-map exit")

local one_success_attack, one_success_pages, one_success_exit,
      one_success_funds, one_success_random, one_success_titles,
      one_success_speakers, one_success_effects = run_haggard(
    "attack", nil, {
        intelligence = 0,
        body = 1,
        scene_id = 5,
        random_values = { [6] = { 0, 5, 0 }, [100] = { 0 } },
    })
eq(one_success_attack, "resolved",
   "ROM 0x23C52 one-success Attack starts the source actor sequence")
eq(one_success_funds, 125,
   "one-success Attack cannot take the no-combat payout")
eq(table.concat(one_success_exit, ","), "stay,fade_83,lock_60",
   "one-success Attack returns through the common same-map exit")
eq(one_success_pages[2].text,
   "<The doctor looks about nervously, then addresses his henchmen.>||" ..
   "You take care of the brash one as well as our friend.||" ..
   "I have other matters to attend to.",
   "ROM 0x121198 one-success Attack result text")
eq(one_success_effects[1],
   "queue_world_effect_5_2_haggard_man_two_bodyguards",
   "ROM 0x1D7050 starts the two-actor map-derived sequence in Scene 5")
eq(#one_success_effects, 1,
   "one-success Attack queues exactly one actor sequence")

local failed_queue_result, failed_queue_pages, failed_queue_exit,
      failed_queue_funds, _, _, _, failed_queue_effects = run_haggard(
    "attack", nil, {
        intelligence = 0,
        body = 1,
        random_values = { [6] = { 0, 0 }, [100] = { 0 } },
        queue_result = "busy",
    })
eq(type(failed_queue_result), "table",
   "busy world effect leaves an explicit source boundary")
eq(failed_queue_result.type, "source_boundary",
   "failed actor sequence is not reported as resolved")
eq(#failed_queue_pages, 1,
   "failed sequence queue does not show its result before actors can spawn")
eq(#failed_queue_exit, 0,
   "failed sequence queue does not apply the same-map exit")
eq(failed_queue_funds, 125,
   "failed sequence queue does not award money")
eq(#failed_queue_effects, 1,
   "failed sequence attempt makes one queue request")

game.location.event_id = "different_event"
eq(RandomEvents.present_haggard_man(), nil,
   "unrelated event cannot enter the escaped-patient handler")
eq(Combat.roll_thresholded_successes(-1, 4, function() return 0 end), nil,
   "negative dice pool rejected")


eq(type(RandomEvents.present_security_guard_stop), "function",
   "ROM 0x20FEC guard prompt has a Lua dialogue task")
local function run_security_guard(choice, has_security_pass, charisma, rolls,
                                 pause_at_dispatch_more, pause_at_badge_more)
    local guard_pages = {}
    local guard_exit = {}
    local guard_choices = {}
    local guard_queued_sequence
    local random_calls = 0
    local random_actions = {}
    local dispatch_more_commands = {}
    local queued_world_effects = {}
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = "security_guard_stop",
        },
        state = {
            ["group_item.security_pass"] = has_security_pass == true,
        },
        modules = {
            combat = Combat,
            world_content_data = {
                world_activity_event_content = {
                    { event_id = "security_guard_stop",
                      opening_text =
                          "<A secuity guard spots you.>||Show me some ID, pal." },
                },
                world_activity_event_choices = {
                    { event_id = "security_guard_stop", choice_order = 1,
                      choice_id = "continue" },
                },
            },
        },
        party = function()
            return {
                [0] = { attributes = { charisma = charisma or 2 } },
            }
        end,
        ui = {
            say_text = function(text, continuation)
                guard_pages[#guard_pages + 1] = {
                    text = text,
                    continuation = continuation,
                    source = game.dialogue.active.source,
                }
                local talk_dispatch_more =
                    pause_at_dispatch_more == true and continuation == true and
                    text:find(
                        "Suspicous, he decides to check in with dispatch...",
                        1, true) ~= nil
                local badge_dispatch_more =
                    pause_at_badge_more == true and continuation == true and
                    text:find(
                        "He checks your ID and calls dispatch to verify it.",
                        1, true) ~= nil
                if talk_dispatch_more or badge_dispatch_more then
                    local marker = badge_dispatch_more and
                        "guard_badge_more" or "guard_dispatch_more"
                    dispatch_more_commands[#dispatch_more_commands + 1] =
                        coroutine.yield(marker)
                end
            end,
            prompt_text = function(menu_id, prompt, options)
                guard_choices[#guard_choices + 1] = {
                    menu_id = menu_id,
                    prompt = prompt,
                    options = options,
                }
                return choice
            end,
        },
        services = {
            call = function(domain, action)
                if domain == "field" and action == "scene_reference" then
                    return "scene_30"
                end
                if domain == "story" and
                   action ==
                       "queue_world_effect_30_1_security_guard_stop_attack" then
                    guard_queued_sequence = action
                    queued_world_effects[#queued_world_effects + 1] = action
                    return "queued"
                end
                if domain == "random" then
                    random_calls = random_calls + 1
                    random_actions[#random_actions + 1] = action
                    local roll = rolls and rolls[random_calls] or nil
                    return roll ~= nil and ("roll_" .. roll) or nil
                end
                if domain == "dialogue_exit" then
                    guard_exit[#guard_exit + 1] = action
                    return "applied"
                end
                return nil
            end,
        },
    }
    local function open_guard()
        return DialogueFramework.open(
            E.DialogueSource.FIELD_ACTIVITY,
            "field_random_events", "present_security_guard_stop")
    end
    local guard_task
    local guard_result
    if pause_at_dispatch_more == true or pause_at_badge_more == true then
        guard_task = coroutine.create(open_guard)
        local resumed, yielded = coroutine.resume(guard_task)
        assert(resumed, tostring(yielded))
        guard_result = yielded
    else
        guard_result = open_guard()
    end
    return guard_result, guard_pages, guard_choices, guard_queued_sequence,
        guard_exit, random_calls, random_actions, guard_task,
        dispatch_more_commands, queued_world_effects
end

if type(RandomEvents.present_security_guard_stop) == "function" then
    local guard_result, guard_pages, guard_choices, guard_queued_sequence,
        guard_exit = run_security_guard("attack")
    eq(type(guard_result), "table",
       "no-ID Attack returns an explicit post-attack source boundary")
    eq(guard_result.type, "source_boundary",
       "partial combat handoff stays visible as a source boundary")
    eq(guard_result.reason:find("0x121290", 1, true) ~= nil, true,
       "Attack boundary cites the ROM weapon-draw record")
    eq(#guard_pages, 2,
       "Attack returns to the shared source text service")
    eq(guard_pages[1].text,
       "<A secuity guard spots you.>||Show me some ID, pal.",
       "guard message uses extracted source text")
    eq(guard_pages[1].continuation, true,
       "guard message waits for the source MORE command")
    eq(guard_pages[1].source, E.DialogueSource.FIELD_ACTIVITY,
       "guard message uses the shared field activity dialogue path")
    eq(guard_pages[2].text,
       "<You quickly pull your weapons and attack!>",
       "ROM 0x121290 is shown after the queued Attack outcome")
    eq(guard_pages[2].continuation, false,
       "ROM weapon-draw text does not add a continuation command")
    eq(guard_pages[2].source, E.DialogueSource.FIELD_ACTIVITY,
       "Attack result stays in the shared field dialogue service")
    eq(#guard_choices, 1,
       "guard MORE opens the no-security-ID choice prompt")
    if guard_choices[1] then
        eq(guard_choices[1].menu_id, "security_guard_no_id",
           "guard prompt has a stable menu ID")
        eq(guard_choices[1].prompt,
           "<Since you don't have a badge, you'll have to come up with something else.>",
           "no-ID prompt uses ROM 0x121278 text")
        eq(#guard_choices[1].options, 2,
           "ROM 0x121278 presents Attack and Talk")
        eq(guard_choices[1].options[1].id, "attack",
           "ROM command 0xD2 selects Attack")
        eq(guard_choices[1].options[2].id, "talk",
           "ROM command 0xD0 selects Talk")
        eq(guard_choices[1].options[1].label, "<Attack!!>",
           "Attack label uses the shared source button format")
        eq(guard_choices[1].options[2].label, "<Talk to him.>",
           "Talk label uses the source button format")
    end
    eq(guard_queued_sequence,
       "queue_world_effect_30_1_security_guard_stop_attack",
       "Attack queues the one-guard source sequence")
    eq(#guard_exit, 0,
       "guard response uses the shared task return lifecycle")

    local talk_result, talk_pages, talk_choices, talk_sequence, _,
        talk_random_calls = run_security_guard(
            "talk", false, 2, { 5, 0, 5, 0 })
    eq(talk_result, "resolved",
       "ROM 0x24432 result above one returns the explanation outcome")
    eq(#talk_choices, 1,
       "Talk follows the same source MORE and no-ID choice lifecycle")
    eq(#talk_pages, 2,
       "successful Talk presents the opening and terminal result page")
    eq(talk_pages[2].text,
       "<You manage to convince him you misplaced it.\n\n" ..
       "He seems satisfied and moves on.>",
       "ROM 0x1212B4 preserves both 0x80-separated sentences from 0x11F378")
    eq(talk_pages[2].text:find(
           "You manage to convince him you misplaced it.", 1, true) ~= nil,
       true, "Talk success keeps the first sentence at 0x11F378")
    eq(talk_pages[2].text:find(
           "He seems satisfied and moves on.", 1, true) ~= nil,
       true, "Talk success keeps the terminal second sentence at 0x11F378")
    eq(talk_pages[2].continuation, false,
       "ROM 0x1212B4 is a terminal text record")
    eq(talk_pages[2].source, E.DialogueSource.FIELD_ACTIVITY,
       "Talk result stays in the shared field dialogue service")
    eq(talk_sequence, nil,
       "successful Talk does not queue a combat actor")
    eq(talk_random_calls, 4,
       "two Charisma dice use the shared exploding-six service")

    local waiting_marker, waiting_pages, _, _, _, waiting_random_calls,
        waiting_random_actions, waiting_task, waiting_more_commands,
        waiting_effects = run_security_guard(
            "talk", false, 2, { 5, 0, 0, 39 }, true)
    eq(waiting_marker, "guard_dispatch_more",
       "one-success Talk pauses at the CE MORE page")
    eq(coroutine.status(waiting_task), "suspended",
       "one-success Talk remains suspended until CE continues MORE")
    eq(#waiting_pages, 2,
       "one-success Talk has not presented a result while MORE is pending")
    eq(waiting_pages[2].continuation, true,
       "the one-success text waits for its MORE command")
    eq(waiting_random_calls, 3,
       "only the Charisma dice have been rolled before MORE resumes")
    eq(table.concat(waiting_random_actions, ","),
       "bounded_6,bounded_6,bounded_6",
       "the tagged dispatch draw is not requested before CE")
    eq(#waiting_effects, 0,
       "guard effect is not queued while the MORE page is pending")

    local resumed, failure_result = coroutine.resume(waiting_task, "CE")
    eq(resumed, true, "CE resumes the one-success Talk continuation")
    eq(failure_result, "resolved",
       "roll 39 resolves through the guard dispatch outcome")
    eq(waiting_more_commands[1], "CE",
       "the dispatch roll follows the CE continuation")
    eq(table.concat(waiting_random_actions, ","),
       "bounded_6,bounded_6,bounded_6,security_guard_dispatch",
       "the post-CE roll uses the tagged security guard dispatch service")
    eq(#waiting_effects, 1,
       "roll 39 queues exactly one guard effect")
    eq(waiting_effects[1],
       "queue_world_effect_30_1_security_guard_stop_attack",
       "roll 39 queues the source guard stop-attack sequence")
    eq(#waiting_pages, 3,
       "roll 39 shows the negative dispatch response after the guard effect")
    eq(waiting_pages[3].text,
       "<Dispatch comes back negative and sounds the alert.>",
       "roll 39 presents the alert response")
    eq(waiting_pages[3].continuation, false,
       "the roll-39 alert response is terminal")

    local safe_result, safe_pages, _, safe_sequence, _, safe_random_calls,
        safe_random_actions, _, _, safe_effects = run_security_guard(
            "talk", false, 2, { 5, 0, 0, 40 })
    eq(safe_result, "resolved",
       "roll 40 resolves through the safe Talk outcome")
    eq(#safe_pages, 3,
       "roll 40 shows its MORE page followed by the safe result")
    eq(safe_pages[2].text,
       "<You barely convince him you have misplaced it.\n\n" ..
       "Suspicous, he decides to check in with dispatch...>",
       "ROM 0x1212A0 preserves its complete two-sentence text")
    eq(safe_pages[2].continuation, true,
       "ROM 0x1212A0 uses the shared MORE continuation at 0xFE5BF")
    eq(safe_pages[3].text,
       "<Dispatch verifies his check and he waves you by.>",
       "ROM 0x121268 presents the verified-dispatch safe result")
    eq(safe_pages[3].continuation, false,
       "roll 40 safe result is terminal")
    eq(safe_pages[3].source, E.DialogueSource.FIELD_ACTIVITY,
       "all Talk pages stay in the shared field dialogue service")
    eq(safe_sequence, nil,
       "roll 40 does not queue a guard effect")
    eq(#safe_effects, 0,
       "roll 40 leaves the world-effect queue unchanged")
    eq(safe_random_calls, 4,
       "one-success Talk consumes its dice and bounded-100 check")
    eq(safe_random_actions[4], "security_guard_dispatch",
       "roll 40 uses the site-tagged dispatch service")

    local zero_success, zero_pages, _, zero_sequence, _, zero_random_calls,
        _, _, _, zero_effects = run_security_guard(
            "talk", false, 1, { 0 })
    eq(zero_success, "resolved",
       "zero Talk successes resolve after queuing the ROM guard encounter")
    eq(#zero_pages, 2,
       "zero-success Talk shows its terminal result after the opening page")
    eq(zero_pages[2].text,
       "<You try a lame story.\n\n" ..
       "You realize this isn't going to work about the time he attacks.>",
       "ROM 0x121298 preserves the complete failed-story text")
    eq(zero_pages[2].continuation, false,
       "ROM 0x121298 has no trailing MORE command")
    eq(zero_pages[2].source, E.DialogueSource.FIELD_ACTIVITY,
       "zero-success text stays in the shared field dialogue service")
    eq(zero_sequence,
       "queue_world_effect_30_1_" ..
           E.WorldEffectSequence.SECURITY_GUARD_STOP_ATTACK,
       "zero successes queue one guard from ROM list 0x1D705C")
    eq(#zero_effects, 1,
       "zero-success Talk queues exactly one world effect")
    eq(zero_effects[1], zero_sequence,
       "the queued zero-success effect uses the guard stop sequence")
    eq(zero_random_calls, 1,
       "zero-success branch consumes one source die")

    local invalid_runner_result = nil
    game.party = function() return { [0] = { attributes = {} } } end
    game.services.call = function(domain, action)
        if domain == "dialogue_exit" then return "applied" end
        return nil
    end
    local invalid_runner_pages = {}
    game.ui.say_text = function(text, continuation)
        invalid_runner_pages[#invalid_runner_pages + 1] = {
            text = text, continuation = continuation,
            source = game.dialogue.active.source,
        }
    end
    game.ui.prompt_text = function() return "talk" end
    invalid_runner_result = DialogueFramework.open(
        E.DialogueSource.FIELD_ACTIVITY,
        "field_random_events", "present_security_guard_stop")
    eq(type(invalid_runner_result), "table",
       "missing slot-zero Charisma stays an explicit source boundary")
    eq(invalid_runner_result.reason:find("Charisma", 1, true) ~= nil, true,
       "missing runner fact identifies the ROM source field")
    eq(#invalid_runner_pages, 1,
       "missing source runner fact does not fabricate a Talk result")

    local badge_pass, badge_pass_pages, badge_pass_choices, _, _,
        badge_pass_random_calls, badge_pass_actions, _, _, badge_pass_effects =
        run_security_guard("attack", true, nil, { 20 })
    eq(badge_pass, "resolved",
       "ROM 0x242FA roll 20 returns the badge-pass branch")
    eq(#badge_pass_choices, 0,
       "badge-present branch does not show no-ID choices")
    eq(badge_pass_pages[2].text,
       "<You show the ID, and he waves you past.>",
       "ROM 0x12124C returns the badge-pass text")
    eq(badge_pass_pages[2].continuation, false,
       "ROM 0x12124C is terminal")
    eq(#badge_pass_effects, 0,
       "badge-pass branch queues no world effect")
    eq(badge_pass_random_calls, 1,
       "badge-pass branch consumes only the ROM 0x2430C roll")
    eq(badge_pass_actions[1], "security_guard_badge",
       "badge roll uses its source-site random tag")

    local badge_wait, badge_wait_pages, _, _, _, badge_wait_calls,
        badge_wait_actions, badge_wait_task, badge_more_commands,
        badge_wait_effects = run_security_guard(
            "attack", true, nil, { 19, 40 }, false, true)
    eq(badge_wait, "guard_badge_more",
       "ROM 0x121254 pauses at its MORE continuation")
    eq(coroutine.status(badge_wait_task), "suspended",
       "badge verification waits for CE before its dispatch roll")
    eq(#badge_wait_pages, 2,
       "the badge check text is visible while MORE is pending")
    eq(badge_wait_pages[2].text,
       "<He checks your ID and calls dispatch to verify it.>",
       "ROM 0x121254 returns the ID verification text")
    eq(badge_wait_pages[2].continuation, true,
       "ROM 0x121254 carries the MORE label at 0x0FE5BF")
    eq(badge_wait_calls, 1,
       "badge dispatch does not roll before CE")
    eq(table.concat(badge_wait_actions, ","), "security_guard_badge",
       "only the ROM 0x2430C badge check ran before CE")
    eq(#badge_wait_effects, 0,
       "badge dispatch queues no effect while MORE is pending")
    local badge_resumed, badge_safe_result =
        coroutine.resume(badge_wait_task, "CE")
    eq(badge_resumed, true, "CE resumes the badge verification route")
    eq(badge_safe_result, "resolved",
       "ROM 0x24392 roll 40 returns its verification success")
    eq(badge_more_commands[1], "CE",
       "the badge dispatch roll follows CE")
    eq(table.concat(badge_wait_actions, ","),
       "security_guard_badge,security_guard_dispatch",
       "badge and dispatch rolls use their source-site tags")
    eq(#badge_wait_pages, 3,
       "the successful dispatch reply follows the badge verification page")
    eq(badge_wait_pages[3].text,
       "<Dispatch verifies his check and he waves you by.>",
       "ROM 0x121268 returns the verified-dispatch text")
    eq(badge_wait_pages[3].continuation, false,
       "ROM 0x121268 is terminal")
    eq(#badge_wait_effects, 0,
       "verified dispatch queues no world effect")
    eq(#badge_wait_actions, 2,
       "the badge path consumes two random calls after CE resumes")

    local badge_alert, badge_alert_pages, _, _, _, badge_alert_calls,
        badge_alert_actions, _, _, badge_alert_effects =
        run_security_guard("attack", true, nil, { 19, 39 })
    eq(badge_alert, "resolved",
       "ROM 0x24392 roll 39 returns the dispatch-alert branch")
    eq(badge_alert_pages[2].text,
       "<He checks your ID and calls dispatch to verify it.>",
       "the alert route first returns the badge verification text")
    eq(badge_alert_pages[2].continuation, true,
       "badge verification waits on its MORE continuation")
    eq(badge_alert_pages[3].text,
       "<Dispatch comes back negative and sounds the alert.>",
       "ROM 0x121270 returns the negative-dispatch text")
    eq(badge_alert_pages[3].continuation, false,
       "ROM 0x121270 is terminal")
    eq(#badge_alert_effects, 1,
       "dispatch roll below 40 queues one guard effect")
    eq(badge_alert_effects[1],
       "queue_world_effect_30_1_security_guard_stop_attack",
       "the alert branch queues ROM list 0x1D705C")
    eq(table.concat(badge_alert_actions, ","),
       "security_guard_badge,security_guard_dispatch",
       "the alert route consumes the badge and dispatch site rolls")
    eq(badge_alert_calls, 2,
       "the alert route consumes exactly two rolls")
end

print(("field_random_events_tests: %d checks"):format(checks))
