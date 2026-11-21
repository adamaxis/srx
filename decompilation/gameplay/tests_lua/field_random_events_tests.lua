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

eq(type(RandomEvents.present_company_man_toothpick), "function",
   "Company Man toothpick random event has a Lua response handler")

local function member(active, quickness, charisma, equipped_armor_id)
    return {
        runner_facts = { lifecycle = { active = active } },
        attributes = { quickness = quickness, charisma = charisma },
        equipped_armor_id = equipped_armor_id,
        equipment = {},
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
    mafia_route, halloweener_route, queue_result, sequence_available)
    local pages = {}
    local route_calls = {}
    local sequence_calls = {}
    local queue_calls = {}
    local random_calls = {}
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
                if domain == "random" and action == "bounded_20" then
                    random_calls[#random_calls + 1] = action
                    return "roll_0"
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
        if sequence_available == false then
            return nil
        end
        return resolve_sequence(scene_id, sequence_id)
    end
    local result = RandomEvents.present_faction_disturbance()
    Encounters.resolve_sequence = resolve_sequence
    return result, pages, route_calls,
           sequence_calls, queue_calls, random_calls
end

local faction_result, faction_pages, faction_services,
      faction_sequence_calls, faction_queue_calls =
    run_faction_disturbance("disabled", "disabled")
eq(faction_result, "resolved",
   "ROM 0x23876 neutral faction route resolves")
eq(#faction_pages, 3,
   "neutral faction route has opening, result and continuation pages")
eq(faction_pages[1].text, "disturbance opening",
   "neutral route uses the extracted event opening")
eq(faction_pages[1].continuation, true,
   "neutral route opening waits for its single MORE command")
-- ROM 0x23876 reaches the no-flag text at 0x11D128 without spawning actors.
eq(faction_pages[2].text,
   "<Impressive figures pass through the parting crowd.||" ..
   "Your first impression is that they are veteran shadowrunners.||" ..
   "Then you see they are wearing gang colors!?>",
   "neutral faction route uses the ROM result record")
-- Record 0x120F90 has a MORE label (0xFE5BF) whose command is record
-- 0x120FA4 (text 0x11D1C9), so the gang-colors page is not terminal.
eq(faction_pages[2].continuation, true,
   "neutral faction result waits for its MORE command")
eq(faction_pages[3].text,
   "<They continue on down the street in an air of confidence, not " ..
   "bothering to harrass those they pass.||" ..
   "Gangs going pro?  You doubt it!!||" ..
   "Shrugging, you continue on your way.>",
   "neutral faction MORE shows the ROM 0x11D1C9 continuation")
eq(faction_pages[3].continuation, false,
   "neutral faction continuation is the terminal text page")
eq(table.concat(faction_services, ","),
   "faction_disturbance_mafia_route_enabled," ..
   "faction_disturbance_halloweener_route_enabled",
   "ROM bit-1 route check precedes the bit-2 route check")
eq(#faction_sequence_calls, 0,
   "no-flag gang-colors text does not resolve an actor sequence")
eq(#faction_queue_calls, 0,
   "no-flag gang-colors text does not queue combat actors")

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

local busy_faction_result, busy_faction_pages, _, busy_sequence_calls,
      busy_queue_calls, busy_random_calls =
    run_faction_disturbance("enabled", "disabled", "busy")
eq(busy_faction_result, "resolved",
   "a failed ROM actor-list start resolves to its fallback event")
eq(#busy_faction_pages, 2,
   "a failed actor-list start shows the ROM holocam fallback")
eq(busy_faction_pages[2].text,
   "<You glimpse someone taking your picture with a holocam.>||" ..
   "The person quickly disappears into the crowd.",
   "failed actor-list start uses the fallback text at ROM 0x11CF74")
eq(busy_faction_pages[2].continuation, false,
   "holocam fallback is a terminal text page")
eq(#busy_sequence_calls, 1,
   "fallback follows an attempted faction actor sequence")
eq(#busy_queue_calls, 1,
   "fallback follows a rejected faction world-effect queue")
eq(table.concat(busy_random_calls, ","), "bounded_20",
   "ROM 0x238C8 consumes its bounded-20 fallback resource selection")

local missing_sequence_result, missing_sequence_pages, _,
      missing_sequence_calls, missing_sequence_queues,
      missing_sequence_random =
    run_faction_disturbance("enabled", "disabled", nil, false)
eq(missing_sequence_result, "resolved",
   "an unavailable ROM actor list resolves to the holocam fallback")
eq(#missing_sequence_pages, 2,
   "an unavailable actor list displays the fallback result page")
eq(missing_sequence_pages[2].text,
   "<You glimpse someone taking your picture with a holocam.>||" ..
   "The person quickly disappears into the crowd.",
   "unavailable actor list uses ROM text 0x11CF74")
eq(#missing_sequence_calls, 1,
   "the missing-list fallback follows the ROM sequence attempt")
eq(#missing_sequence_queues, 0,
   "an unavailable actor list does not queue a nonexistent world effect")
eq(table.concat(missing_sequence_random, ","), "bounded_20",
   "missing-list fallback preserves the ROM resource-selection draw")

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
    local funds = configuration.initial_nuyen or 125
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
        nuyen = function()
            if configuration.missing_nuyen then return nil end
            return funds
        end,
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
                eq(options[1].label, "Attack!!",
                   "ROM 0x103A9A doctor Attack label")
                eq(options[2].label,
                   "Leave him alone or you'll be sorry!",
                   "ROM 0x103BFE doctor Threaten label")
                eq(options[3].label, "Help them.",
                   "ROM 0x103ADE doctor Help label")
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

local function run_watcher(choice, configuration)
    configuration = configuration or {}
    local dialogue_pages = {}
    local exit_calls = {}
    local random_calls = {}
    local prompt_titles = {}
    local prompt_options = {}
    local prompt_speakers = {}
    local queued_world_effects = {}
    local random_values = configuration.random_values or { [6] = { 0 } }
    local random_indices = {}
    local funds = configuration.initial_nuyen or 125
    local nuyen_writes = 0
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = "phantasmal_watcher",
            scene_id = configuration.scene_id or 5,
        },
        modules = {
            combat = Combat,
            world_content_data = {
                world_activity_event_content = {
                    {
                        event_id = "phantasmal_watcher",
                        opening_text =
                            "<The shadowy image of a watcher, a phantasmal " ..
                            "magical messenger, materializes in front of you.>" ..
                            "||Come quickly!||My master needs help!",
                    },
                },
                world_activity_event_choices = {
                    {
                        event_id = "phantasmal_watcher",
                        choice_order = 1,
                        choice_id = "accept_watcher_request",
                        label = "Take me to him.  I'll help.",
                    },
                    {
                        event_id = "phantasmal_watcher",
                        choice_order = 2,
                        choice_id = "insult",
                        label = "Slot off, frag-face!!!",
                    },
                },
            },
        },
        party = function()
            return {
                [0] = { attributes = { body = configuration.body or 1 } },
            }
        end,
        nuyen = function() return funds end,
        set_nuyen = function(value)
            funds = value
            nuyen_writes = nuyen_writes + 1
        end,
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
                eq(menu_id, "phantasmal_watcher", "watcher menu id")
                prompt_titles[#prompt_titles + 1] = prompt
                prompt_options[#prompt_options + 1] = options
                prompt_speakers[#prompt_speakers + 1] = {
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
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
                    if bound == nil then return "invalid_random_result" end
                    random_indices[bound] = (random_indices[bound] or 0) + 1
                    local values = random_values[bound]
                    local value = type(values) == "table" and
                        values[random_indices[bound]] or nil
                    if value == nil then return "invalid_random_result" end
                    return "roll_" .. tostring(value)
                end
                eq(domain, "dialogue_exit",
                   "watcher exit uses the common executor service")
                exit_calls[#exit_calls + 1] = action
                return "applied"
            end,
        },
    }
    local task_result = DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "field_random_events", "present_phantasmal_watcher")
    return task_result, dialogue_pages, exit_calls, funds, random_calls,
        prompt_titles, prompt_options, prompt_speakers, queued_world_effects,
        nuyen_writes
end

local function run_company_man_toothpick(choice, configuration)
    configuration = configuration or {}
    local pages = {}
    local exit_calls = {}
    local random_calls = {}
    local prompt_titles = {}
    local prompt_options = {}
    local prompt_speakers = {}
    local random_values = configuration.random_values or { [6] = {} }
    local random_indices = {}
    local party = configuration.party or {
        [0] = member(true, 2, 3),
        [1] = member(true, 4, 9),
        [2] = member(false, 255, 255),
    }
    local alarm_calls = 0
    local karma_calls = 0
    local resource_variant_calls = 0
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = "company_man_toothpick",
            scene_id = 26,
        },
        modules = {
            combat = Combat,
            world_content_data = {
                world_activity_event_content = {
                    {
                        event_id = "company_man_toothpick",
                        opening_text =
                            "<A Company Man walks up and flicks a " ..
                            "toothpick at you.>",
                    },
                },
                world_activity_event_choices = {
                    { event_id = "company_man_toothpick", choice_order = 1,
                      choice_id = "attack", label = "<Attack!!>" },
                    { event_id = "company_man_toothpick", choice_order = 2,
                      choice_id = "insult",
                      label = "Slot off, frag-face!!!" },
                    { event_id = "company_man_toothpick", choice_order = 3,
                      choice_id = "leave", label = "<Walk away.>" },
                },
            },
        },
        party = function() return party end,
        ui = {
            say_text = function(text, continuation, speaker_id, text_profile)
                pages[#pages + 1] = {
                    text = text,
                    continuation = continuation,
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
            end,
            prompt_text = function(menu_id, prompt, options,
                                   speaker_id, text_profile)
                eq(menu_id, "company_man_toothpick",
                   "Company Man menu id")
                prompt_titles[#prompt_titles + 1] = prompt
                prompt_options[#prompt_options + 1] = options
                prompt_speakers[#prompt_speakers + 1] = {
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
                return choice
            end,
        },
        services = {
            call = function(domain, action)
                if domain == "random" then
                    random_calls[#random_calls + 1] = action
                    eq(action, "bounded_6", "Company Man source die")
                    random_indices[6] = (random_indices[6] or 0) + 1
                    local value = random_values[6] and
                        random_values[6][random_indices[6]] or nil
                    return value ~= nil and ("roll_" .. value) or nil
                end
                if domain == "field" and action == "raise_alarm" then
                    alarm_calls = alarm_calls + 1
                    return "applied"
                end
                if domain == "party" and action == "award_karma_1" then
                    karma_calls = karma_calls + 1
                    return "applied"
                end
                if domain == "field" and action == "resource_variant" then
                    resource_variant_calls = resource_variant_calls + 1
                    if configuration.resource_variant_unavailable then
                        return nil
                    end
                    return configuration.resource_variant or "third"
                end
                eq(domain, "dialogue_exit",
                   "Company Man exit uses the common executor service")
                exit_calls[#exit_calls + 1] = action
                return "applied"
            end,
        },
    }
    local result = DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "field_random_events", "present_company_man_toothpick")
    return result, pages, exit_calls, alarm_calls, karma_calls,
        random_calls, prompt_titles, prompt_options, prompt_speakers,
        resource_variant_calls
end

local function run_company_man_confrontation(choice, configuration)
    configuration = configuration or {}
    local pages = {}
    local exit_calls = {}
    local random_calls = {}
    local prompt_titles = {}
    local prompt_options = {}
    local prompt_speakers = {}
    local prompt_call_count = 0
    local random_values = configuration.random_values or {}
    local random_indices = {}
    local prompt_choices = configuration.prompt_choices or {}
    local story_calls = {}
    local heat_level = configuration.heat_level or 0
    local nuyen = configuration.nuyen or 1000
    local party = configuration.party or { [0] = member(true, 2, 2) }
    local patrol_profiles = {
        { actor_name = "lone11", profile_id = "lone11_k05_item03" },
        { actor_name = "lone12", profile_id = "lone12_k05_item07" },
        {
            actor_name = "lone detectiv",
            profile_id = "lone_detectiv_k05_item07",
        },
    }
    local patrol_actor_rows = {}
    for order, profile in ipairs(patrol_profiles) do
        patrol_actor_rows[#patrol_actor_rows + 1] = {
            scene_id = 3,
            sequence_order = order - 1,
            sequence_id = "patrol_profile_catalog",
            actor_name = profile.actor_name,
            profile_id = profile.profile_id,
            visual_profile_id = E.ActorVisualProfile.HUMAN_GUARD,
            action_kind = "item",
            action_id = 7,
            actor_facts = { interaction = {} },
            physical_meter = 10,
            palette_line = 1,
            sprite_stem = "actor-kind-5-palette-1",
        }
    end
    game = {
        location = {
            type = "field_map",
            id = "random_field_event",
            event_id = configuration.event_id or
                "company_man_confrontation",
            scene_id = 3,
        },
        state = {
            vid_phone_affiliation_mafia =
                configuration.mafia_affiliation == true,
            ["group_item.weapon_permit"] =
                configuration.weapon_permit == true,
        },
        modules = {
            combat = Combat,
            world_content_data = {
                world_activity_event_content = {
                    {
                        event_id = "company_man_confrontation",
                        opening_text =
                            "<A Company Man storms up to you.>||" ..
                            "Hey, you can't get away with this!||" ..
                            "Don't think I don't recognize you!",
                    },
                    {
                        event_id = "lone_star_patrol_stop",
                        opening_text =
                            "<Crowds part as you are approached by a " ..
                            "Lone Star patrol.>||A moment, citizen.  " ..
                            "We want a word with you.",
                    },
                },
                world_activity_event_choices = {
                    { event_id = "company_man_confrontation",
                      choice_order = 1, choice_id = "attack",
                      label = "<Attack!!>" },
                    { event_id = "company_man_confrontation",
                      choice_order = 2, choice_id = "talk",
                      label = "<Talk to him.>" },
                    { event_id = "company_man_confrontation",
                      choice_order = 3, choice_id = "leave",
                      label = "<Walk away.>" },
                    { event_id = "lone_star_patrol_stop",
                      choice_order = 1, choice_id = "attack",
                      label = "<Attack!!>" },
                    { event_id = "lone_star_patrol_stop",
                      choice_order = 2, choice_id = "talk",
                      label = "<Talk to them.>" },
                    { event_id = "lone_star_patrol_stop",
                      choice_order = 3, choice_id = "run_away",
                      label = "<Run away!>" },
                },
                dynamic_world_activity_actors = patrol_actor_rows,
            },
        },
        party = function() return party end,
        ui = {
            say_text = function(text, continuation, speaker_id, text_profile)
                pages[#pages + 1] = {
                    text = text,
                    continuation = continuation,
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
            end,
            prompt_text = function(menu_id, prompt, options,
                                   speaker_id, text_profile)
                assert(menu_id == "company_man_confrontation" or
                       menu_id == "lone_star_patrol_stop" or
                       menu_id == "lone_star_suspect_id" or
                       menu_id == "lone_star_illegal_weapon",
                       "Company Man and Lone Star menu ids")
                prompt_titles[#prompt_titles + 1] = prompt
                prompt_options[#prompt_options + 1] = options
                prompt_speakers[#prompt_speakers + 1] = {
                    speaker_id = speaker_id,
                    text_profile = text_profile,
                }
                prompt_call_count = prompt_call_count + 1
                return prompt_choices[prompt_call_count] or choice
            end,
        },
        services = {
            call = function(domain, action)
                if domain == "random" then
                    local bound = tonumber(
                        type(action) == "string" and
                            action:match("^bounded_(%d+)$") or "")
                    assert(bound == 0 or bound == 2 or bound == 6 or
                           bound == 8 or bound == 10 or bound == 12 or
                           bound == 20 or bound == 100,
                           "Company Man confrontation random bound")
                    random_calls[#random_calls + 1] = action
                    random_indices[bound] = (random_indices[bound] or 0) + 1
                    local values = random_values[bound]
                    local value = values and values[random_indices[bound]]
                    return value ~= nil and ("roll_" .. value) or nil
                end
                if domain == "field" and action == "scene_reference" then
                    return "scene_3"
                end
                if domain == "story" then
                    story_calls[#story_calls + 1] = action
                    if action == "get_heat_level" then
                        return "level_" .. heat_level
                    end
                    local new_heat = tonumber(
                        action:match("^set_heat_level_(%d+)$"))
                    if new_heat ~= nil then
                        heat_level = new_heat
                        return "applied"
                    end
                    if type(action) == "string" and
                       action:match("^queue_world_effect_3_%d+_[%w_]+$") then
                        return configuration.queue_world_effect_result or
                            "queued"
                    end
                    return nil
                end
                eq(domain, "dialogue_exit",
                   "Company Man confrontation uses the shared exit service")
                exit_calls[#exit_calls + 1] = action
                return "applied"
            end,
        },
        nuyen = function() return nuyen end,
        set_nuyen = function(value) nuyen = value end,
    }
    local result = DialogueFramework.open(
        E.DialogueSource.RANDOM_ENCOUNTER,
        "field_random_events",
        configuration.task_name or "present_company_man_confrontation")
    return result, pages, exit_calls, random_calls, prompt_titles,
        prompt_options, prompt_speakers, story_calls, heat_level, nuyen
end

-- ROM entry 0x20F5A is a distinct Company Man encounter from the 0x210B4
-- toothpick event. Cover each choice's no-actor outcome and explicit unresolved
-- actor/patrol handoffs at the source branch conditions.
do
local route = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "field_random_events.present_company_man_confrontation")
eq(route.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
   "Company Man confrontation returns to its source field scene")
eq(route.exit_behavior, E.DialogueExitBehavior.STAY,
   "Company Man confrontation uses the shared stay exit")
eq(route.transition, E.DialogueTransition.FADE,
   "Company Man confrontation returns through the shared fade")
eq(route.transition_frames,
   E.DialogueTransitionFrames.HAGGARD_BLACK_INTERVAL,
   "Company Man confrontation preserves the measured event fade interval")
eq(route.input_lock, E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
   "Company Man confrontation uses the world-activity input lock")
eq(route.input_lock_frames, E.DialogueInputLockFrames.WORLD_ACTIVITY,
   "Company Man confrontation preserves the world-activity lock duration")

local patrol_route = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "field_random_events.present_lone_star_patrol_stop")
eq(patrol_route.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
   "Lone Star prompt returns to its source field")
eq(patrol_route.exit_behavior, E.DialogueExitBehavior.STAY,
   "Lone Star prompt uses the shared stay exit")
eq(patrol_route.input_lock,
   E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
   "Lone Star prompt uses the world-activity input lock")

local patrol_attack, patrol_attack_pages, _, _, patrol_titles, patrol_options,
      _, patrol_story_calls, patrol_heat =
    run_company_man_confrontation("attack", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "attack" },
        heat_level = 7,
    })
eq(patrol_attack, "resolved", "Lone Star Attack starts its source sequence")
eq(patrol_titles[1],
   "<Crowds part as you are approached by a Lone Star patrol.>||" ..
   "A moment, citizen.  We want a word with you.",
   "ROM 0x20F12 uses its extracted patrol prompt")
eq(#patrol_options[1], 3, "Lone Star prompt has three source choices")
eq(patrol_options[1][1].id, "attack", "Lone Star Attack command")
eq(patrol_options[1][2].id, "talk", "Lone Star Talk command")
eq(patrol_options[1][3].id, "run_away", "Lone Star Run command")
eq(patrol_attack_pages[1].text,
   "<The Lone Star forces draw their weapons with well-trained efficiency.>",
   "ROM 0x23412 successful actor-list result")
eq(patrol_attack_pages[1].continuation, true,
   "patrol fight lead-in waits for the source page to be acknowledged")
eq(patrol_heat, 8, "ROM 0x23412 adds two heat capped at eight")
eq(table.concat(patrol_story_calls, ","),
   "get_heat_level,queue_world_effect_3_3_lone_star_patrol_attack,set_heat_level_8",
   "patrol attack queues its three source actors before raising heat")
local patrol_attack_sequence = Encounters.resolve_sequence(
    3, E.WorldEffectSequence.LONE_STAR_PATROL_ATTACK)
eq(patrol_attack_sequence.kind, "dynamic_world_effect_sequence",
   "ROM 0x1D6FDC resolves to a typed map-placed actor sequence")
eq(patrol_attack_sequence.placement_policy,
   E.WorldEffectPlacementPolicy.MAP_DERIVED_CANDIDATES,
   "ROM 0x23412 uses map-derived actor placement")
eq(#patrol_attack_sequence.actors, 3,
   "ROM 0x1D6FDC contains three patrol actors")
eq(patrol_attack_sequence.actors[1].profile_id, "lone11_k05_item03",
   "patrol sequence actor one is Lone 11")
eq(patrol_attack_sequence.actors[2].profile_id, "lone12_k05_item07",
   "patrol sequence actor two is Lone 12")
eq(patrol_attack_sequence.actors[3].profile_id,
   "lone_detectiv_k05_item07", "patrol sequence actor three is detective")

do
local function run_patrol_run(party, heat, dice, queue_result)
    return run_company_man_confrontation("run_away", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "run_away" },
        party = party,
        heat_level = heat,
        random_values = { [6] = dice },
        queue_world_effect_result = queue_result,
    })
end

local caught_run, caught_pages, _, caught_random, _, caught_choices, _,
      caught_story, caught_heat = run_patrol_run(
    { [0] = member(true, 4, 2), [1] = member(false, 0, 0),
      [2] = member(false, 0, 0) },
    3, { 0, 0, 0, 0 })
eq(caught_run, "resolved", "patrol Run resolves the lowest chase band")
eq(caught_pages[1].text,
   "<The detectives easily overtake you and begin firing.>",
   "0-1 chase successes return ROM text 0x11D3C1")
eq(caught_pages[1].continuation, false,
   "patrol Run result is a terminal result page")
eq(#caught_choices[1], 3, "patrol Run uses the source three-choice menu")
eq(table.concat(caught_random, ","),
   "bounded_6,bounded_6,bounded_6,bounded_6",
   "one active runner supplies four Quickness trials")
eq(table.concat(caught_story, ","),
   "get_heat_level,set_heat_level_4," ..
   "queue_world_effect_3_1_lone_star_patrol_run_detective",
   "Run raises heat before its one-detective queue")
eq(caught_heat, 4, "patrol Run raises heat once")
local caught_sequence = Encounters.resolve_sequence(
    3, E.WorldEffectSequence.LONE_STAR_PATROL_RUN_DETECTIVE)
eq(#caught_sequence.actors, 1, "low Run result resolves one source actor")
eq(caught_sequence.actors[1].profile_id, "lone_detectiv_k05_item07",
   "0x1D6FE4 resolves the Lone Star detective")
eq(caught_sequence.placement_policy,
   E.WorldEffectPlacementPolicy.SCREEN_EDGE_CANDIDATES,
   "Run actor list uses the source D0-zero edge placement")

local partial_run, partial_pages, _, partial_random, _, _, _, partial_story,
      partial_heat = run_patrol_run(
    { [0] = member(true, 4, 2, "lined_duster"),
      [1] = member(false, 0, 0), [2] = member(false, 0, 0) },
    7, { 4, 4, 0, 0 })
eq(partial_run, "resolved", "patrol Run resolves the partial chase band")
eq(partial_pages[1].text,
   "<You outrun some of them, but the others appear to be in better shape.||" ..
   "They overtake you and open fire.>",
   "2-3 successes at lined-duster target 5 return ROM text 0x11D3F8")
eq(table.concat(partial_random, ","),
   "bounded_6,bounded_6,bounded_6,bounded_6",
   "lined-duster check uses four live Quickness trials")
eq(table.concat(partial_story, ","),
   "get_heat_level,set_heat_level_8," ..
   "queue_world_effect_3_2_lone_star_patrol_run_lone12_and_detective",
   "partial chase queues list 0x1D6FE0 after heat caps")
eq(partial_heat, 8, "patrol Run heat increment caps at eight")
local partial_sequence = Encounters.resolve_sequence(
    3, E.WorldEffectSequence.LONE_STAR_PATROL_RUN_LONE12_AND_DETECTIVE)
eq(#partial_sequence.actors, 2,
   "mid Run result resolves both source actors")
eq(partial_sequence.actors[1].profile_id, "lone12_k05_item07",
   "0x1D6FE0 actor one is Lone 12")
eq(partial_sequence.actors[2].profile_id, "lone_detectiv_k05_item07",
   "0x1D6FE0 actor two is the detective")
eq(partial_sequence.placement_policy,
   E.WorldEffectPlacementPolicy.SCREEN_EDGE_CANDIDATES,
   "partial Run uses source edge placement")

local escape_run, escape_pages, _, escape_random, _, _, _, escape_story,
      escape_heat = run_patrol_run(
    { [0] = member(true, 4, 2), [1] = member(false, 0, 0),
      [2] = member(false, 0, 0) },
    7, { 3, 3, 3, 3 })
eq(escape_run, "resolved", "four chase successes escape")
eq(escape_pages[1].text,
   "<You easily outrun the slothfull patrol.>",
   "4+ chase successes return ROM text 0x11D462")
eq(table.concat(escape_random, ","),
   "bounded_6,bounded_6,bounded_6,bounded_6",
   "four successes consume the complete source dice pool")
eq(table.concat(escape_story, ","), "get_heat_level,set_heat_level_8",
   "successful escape increments capped heat without queuing actors")
eq(escape_heat, 8, "successful escape retains the heat cap")

local failed_queue_run, failed_queue_pages, _, _, _, _, _, failed_queue_story,
      failed_queue_heat = run_patrol_run(
    { [0] = member(true, 4, 2), [1] = member(false, 0, 0),
      [2] = member(false, 0, 0) },
    4, { 0, 0, 0, 0 }, "no_actor_slots")
eq(failed_queue_run, "resolved", "actor placement failure uses the ROM fallback")
eq(failed_queue_pages[1].text,
   "<You easily outrun the slothfull patrol.>",
   "failed 0x1E1FC list start returns the shared escape text")
eq(table.concat(failed_queue_story, ","),
   "get_heat_level,set_heat_level_5," ..
   "queue_world_effect_3_1_lone_star_patrol_run_detective",
   "failed sequence keeps the pre-roll heat update")
eq(failed_queue_heat, 5, "failed actor queue still retains Run heat")

-- The ROM only reads the lead runner's armor selector. Source armor indices
-- 1-4 and 7 use target 4; lined duster (5) and light combat armor (6) use 5
-- and 6. Four face-4 dice distinguish those thresholds across the catalog.
local run_armor_cases = {
    { "armor_vest", true },
    { "armor_clothing", true },
    { "armor_jacket", true },
    { "vest_with_plates", true },
    { "lined_duster", false },
    { "light_combat_armor", false },
    { "heavy_combat_armor", true },
}
for _, armor_case in ipairs(run_armor_cases) do
    local result, pages = run_patrol_run(
        { [0] = member(true, 4, 2, armor_case[1]),
          [1] = member(false, 0, 0), [2] = member(false, 0, 0) },
        0, { 3, 3, 3, 3 })
    eq(result, "resolved", "patrol Run resolves for armor " .. armor_case[1])
    eq(pages[1].text,
       armor_case[2] and "<You easily outrun the slothfull patrol.>" or
           "<The detectives easily overtake you and begin firing.>",
       "ROM 0x237A2/0x237B0 armor target branch " .. armor_case[1])
end

local support_armor_run, support_armor_pages = run_patrol_run(
    { [0] = member(true, 4, 2, "armor_vest"),
      [1] = member(true, 4, 2, "light_combat_armor"),
      [2] = member(false, 0, 0) },
    0, { 3, 3, 3, 3 })
eq(support_armor_run, "resolved",
   "support armor does not replace the lead armor modifier")
eq(support_armor_pages[1].text,
   "<You easily outrun the slothfull patrol.>",
   "ROM 0x237A2 reads only runner slot zero armor")

local two_runner_run, two_runner_pages, _, two_runner_random = run_patrol_run(
    { [0] = member(true, 3, 2), [1] = member(true, 4, 2),
      [2] = member(false, 0, 0) },
    0, { 3, 3, 0 })
eq(two_runner_run, "resolved", "two-runner chase resolves")
eq(two_runner_pages[1].text,
   "<You outrun some of them, but the others appear to be in better shape.||" ..
   "They overtake you and open fire.>",
   "two-runner Quickness average floors 3.5 to 3 trials")
eq(#two_runner_random, 3, "two-runner average drives three dice")

local three_runner_run, three_runner_pages, _, three_runner_random =
    run_patrol_run(
        { [0] = member(true, 3, 2), [1] = member(true, 4, 2),
          [2] = member(true, 5, 2) },
        0, { 3, 3, 3, 0 })
eq(three_runner_run, "resolved", "three-runner chase resolves")
eq(three_runner_pages[1].text,
   "<You outrun some of them, but the others appear to be in better shape.||" ..
   "They overtake you and open fire.>",
   "three-runner average Quickness 4 controls the dice pool")
eq(#three_runner_random, 4, "three-runner average drives four dice")

local inactive_run, inactive_pages, _, inactive_random = run_patrol_run(
    { [0] = member(true, 3, 2), [1] = member(false, 99, 2),
      [2] = member(true, 5, 2) },
    0, { 3, 3, 3, 3 })
eq(inactive_run, "resolved", "inactive runner does not block party scan")
eq(inactive_pages[1].text,
   "<You easily outrun the slothfull patrol.>",
   "inactive Quickness does not enter the active-party average")
eq(#inactive_random, 4, "noncontiguous active records average both runners")

local zero_quickness_run, zero_quickness_pages, _, zero_quickness_random =
    run_patrol_run(
        { [0] = member(true, 0, 2), [1] = member(false, 0, 0),
          [2] = member(false, 0, 0) },
        0, { 0 })
eq(zero_quickness_run, "resolved", "zero Quickness still runs the source check")
eq(zero_quickness_pages[1].text,
   "<The detectives easily overtake you and begin firing.>",
   "0x0DBA performs one trial for a zero dice pool")
eq(#zero_quickness_random, 1,
   "zero Quickness preserves the source minimum random draw")

local exploding_armor_run, exploding_armor_pages, _, exploding_armor_random =
    run_patrol_run(
        { [0] = member(true, 2, 2, "light_combat_armor"),
          [1] = member(false, 0, 0), [2] = member(false, 0, 0) },
        0, { 5, 0, 5, 0 })
eq(exploding_armor_run, "resolved",
   "light-combat armor chase accepts exploding sixes")
eq(exploding_armor_pages[1].text,
   "<You outrun some of them, but the others appear to be in better shape.||" ..
   "They overtake you and open fire.>",
   "two target-6 successes select the partial pursuit")
eq(#exploding_armor_random, 4,
   "each six retries the same source trial before the next die")

local nested_run, nested_run_pages, _, nested_run_random, nested_run_titles,
      _, _, nested_run_story, nested_run_heat =
    run_company_man_confrontation("talk", {
        prompt_choices = { "talk", "run_away" },
        random_values = { [6] = { 0, 0, 3, 3, 3, 3 } },
        party = { [0] = member(true, 4, 2), [1] = member(false, 0, 0),
                  [2] = member(false, 0, 0) },
        heat_level = 2,
    })
eq(nested_run, "resolved",
   "Company Man nested patrol Run uses the shared chase handler")
eq(#nested_run_titles, 2,
   "Company Man Talk-zero path opens the nested patrol prompt")
eq(nested_run_pages[1].text,
   "<You easily outrun the slothfull patrol.>",
   "nested patrol Run resolves the ROM escape page")
eq(#nested_run_random, 6,
   "nested prompt and chase preserve both source random sequences")
eq(table.concat(nested_run_story, ","),
   "get_heat_level,set_heat_level_3",
   "nested Run increments heat once before the escape roll")
eq(nested_run_heat, 3, "nested Run retains its heat update")
end

local patrol_failed, patrol_failed_pages, _, _, _, _, _, patrol_failed_calls,
      patrol_failed_heat =
    run_company_man_confrontation("attack", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "attack" },
        queue_world_effect_result = "no_actor_slots",
        heat_level = 8,
    })
eq(patrol_failed, "resolved", "failed patrol sequence returns its ROM text")
eq(patrol_failed_pages[1].text,
   "<As you are about to jump them, they get a call on the radio and " ..
   "hastily leave.>", "ROM 0x23412 zero-result response")
eq(patrol_failed_heat, 8,
   "failed patrol sequence does not change Lone Star heat")
eq(table.concat(patrol_failed_calls, ","),
   "get_heat_level,queue_world_effect_3_3_lone_star_patrol_attack",
   "failed sequence does not write heat")

local attack_flee, attack_flee_pages, attack_flee_exit,
      attack_flee_random, attack_flee_titles, attack_flee_options,
      attack_flee_speakers =
    run_company_man_confrontation("attack", {
        random_values = { [100] = { 49 } },
    })
eq(attack_flee, "resolved", "Company Man confrontation Attack 49 resolves")
eq(attack_flee_titles[1],
   "<A Company Man storms up to you.>||" ..
   "Hey, you can't get away with this!||" ..
   "Don't think I don't recognize you!",
   "Company Man confrontation prompt uses its extracted opening")
eq(#attack_flee_options[1], 3,
   "Company Man confrontation prompt has three choices")
eq(attack_flee_options[1][2].id, "talk",
   "Company Man confrontation second choice is Talk")
eq(attack_flee_options[1][3].id, "leave",
   "Company Man confrontation third choice is Walk away")
eq(attack_flee_speakers[1].speaker_id, "company_man_confrontation",
   "Company Man confrontation uses its source-bound portrait speaker")
eq(attack_flee_speakers[1].text_profile, "field.world_event",
   "Company Man confrontation uses the event text profile")
eq(#attack_flee_pages, 1, "Attack 49 has one result page")
eq(attack_flee_pages[1].text,
   "<As you move to attack, the man turns white and runs away.>",
   "ROM 0x238EA Attack 49 result")
eq(attack_flee_pages[1].speaker_id, "company_man_confrontation",
   "Attack result preserves the source-bound speaker")
eq(table.concat(attack_flee_random, ","), "bounded_100",
   "Attack uses one bounded-100 source roll")
eq(table.concat(attack_flee_exit, ","), "restore_field_context_stay",
   "Attack flee returns through the common field exit")

local talk_apology, talk_apology_pages, _, talk_apology_random =
    run_company_man_confrontation("talk", {
        random_values = { [6] = { 4, 4 } },
    })
eq(talk_apology, "resolved",
   "Talk with two target-5 successes resolves")
eq(talk_apology_pages[1].text,
   "Wait... You aren't Mr. Johnson, are you?||" ..
   "I am very sorry to have bothered you.||" ..
   "<The confused man scurries off.>",
   "ROM 0x23920 multi-success Talk result")
eq(table.concat(talk_apology_random, ","), "bounded_6,bounded_6",
   "Talk rolls the first runner's two Charisma dice against target 5")

do
local talk_zero, talk_zero_pages, _, talk_zero_random,
      talk_zero_titles, talk_zero_options, _, talk_zero_story,
      talk_zero_heat =
    run_company_man_confrontation("talk", {
        random_values = { [6] = { 0, 0, 3, 3 } },
        prompt_choices = { "talk", "run_away" },
    })
eq(talk_zero, "resolved", "Talk zero-success patrol Run resolves")
eq(#talk_zero_titles, 2,
   "Talk zero-success presents the nested Lone Star prompt")
eq(talk_zero_options[2][3].id, "run_away",
   "Talk zero-success exposes the patrol Run choice")
eq(talk_zero_pages[1].text,
   "<You outrun some of them, but the others appear to be in better shape.||" ..
   "They overtake you and open fire.>",
   "nested Company Man Run reaches the shared partial chase result")
eq(#talk_zero_random, 4,
   "nested Run continues after the two Company Man Talk dice")
eq(table.concat(talk_zero_story, ","),
   "get_heat_level,set_heat_level_1," ..
   "queue_world_effect_3_2_lone_star_patrol_run_lone12_and_detective",
   "nested patrol Run uses the shared heat and actor queue")
eq(talk_zero_heat, 1, "nested patrol Run increments heat once")
end

local talk_zero_attack, talk_zero_attack_pages, _, _,
      _, _, _, talk_zero_attack_calls, talk_zero_attack_heat =
    run_company_man_confrontation("talk", {
        random_values = { [6] = { 0, 0 } },
        prompt_choices = { "talk", "attack" },
        heat_level = 7,
    })
eq(talk_zero_attack, "resolved",
   "Talk zero-success routes nested patrol Attack through 0x23412")
eq(talk_zero_attack_pages[1].text,
   "<The Lone Star forces draw their weapons with well-trained efficiency.>",
   "nested Talk patrol Attack uses the shared result record")
eq(talk_zero_attack_heat, 8,
   "nested Talk patrol Attack raises heat after the list queues")
eq(table.concat(talk_zero_attack_calls, ","),
   "get_heat_level,queue_world_effect_3_3_lone_star_patrol_attack,set_heat_level_8",
   "nested Talk patrol Attack uses the shared queue and heat path")

local talk_one = run_company_man_confrontation("talk", {
    random_values = { [6] = { 4, 0 } },
})
eq(type(talk_one), "table", "Talk one-success actor path remains explicit")
eq(talk_one.type, "source_boundary",
   "Talk one-success does not silently skip actor list 0x1D7074")

local attack_gun_draw = run_company_man_confrontation("attack", {
    random_values = { [100] = { 50 } },
})
eq(type(attack_gun_draw), "table",
   "Attack 50 actor handoff remains explicit")
eq(attack_gun_draw.type, "source_boundary",
   "Attack 50 does not fabricate the source pedestrian sequence")

local leave_follow, leave_follow_pages, _, leave_follow_random =
    run_company_man_confrontation("leave", {
        random_values = { [100] = { 39 } },
    })
eq(leave_follow, "resolved", "Walk Away 39 resolves")
eq(leave_follow_pages[1].text,
   "<The man follows you for a while, yelling at you.||" ..
   "He eventually moves on.>", "ROM 0x23978 Walk Away 39 result")
eq(table.concat(leave_follow_random, ","), "bounded_100",
   "Walk Away uses one bounded-100 source roll")

local leave_gun_draw = run_company_man_confrontation("leave", {
    random_values = { [100] = { 40 } },
})
eq(type(leave_gun_draw), "table",
   "Walk Away 40 actor handoff remains explicit")
eq(leave_gun_draw.type, "source_boundary",
   "Walk Away 40 does not fabricate actor list 0x1D7074")

local leave_patrol, leave_patrol_pages, _, leave_patrol_random,
      leave_patrol_titles, leave_patrol_options, _, leave_patrol_story,
      leave_patrol_heat =
    run_company_man_confrontation("leave", {
        random_values = { [100] = { 80 }, [12] = { 0 } },
        prompt_choices = { "leave", "talk" },
    })
eq(leave_patrol, "resolved",
   "Walk Away 80 patrol Talk resolves the shared compliance check")
eq(#leave_patrol_titles, 2,
   "Walk Away 80 presents the nested Lone Star prompt")
eq(leave_patrol_options[2][2].id, "talk",
   "Walk Away 80 exposes the patrol Talk choice")
eq(leave_patrol_pages[1].text,
   "<Keeping your cool, you answer all of their questions convincingly.||" ..
   "Finally, Lone Star continues on its way.>",
   "Walk Away 80 patrol Talk uses ROM text 0x11D637")
eq(table.concat(leave_patrol_random, ","), "bounded_100,bounded_12",
   "Walk Away 80 patrol Talk samples its general 0-11 check")
eq(table.concat(leave_patrol_story, ","),
   "get_heat_level,set_heat_level_0",
   "successful compliance lowers heat, clamped at zero")
eq(leave_patrol_heat, 0,
   "successful compliance preserves zero heat")

local comply_party = {
    [0] = member(true, 2, 2),
    [1] = member(false, 0, 0),
    [2] = member(false, 0, 0),
}
local comply_general, comply_general_pages, _, comply_general_random,
      _, _, _, comply_general_story, comply_general_heat =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk" },
        random_values = { [12] = { 4 } },
        heat_level = 3,
        party = comply_party,
    })
eq(comply_general, "resolved",
   "ROM 0x23482 adjusted roll at or below Charisma releases the party")
eq(comply_general_pages[1].text,
   "<Keeping your cool, you answer all of their questions convincingly.||" ..
   "Finally, Lone Star continues on its way.>",
   "successful compliance uses the extracted release record")
eq(table.concat(comply_general_random, ","), "bounded_12",
   "general compliance uses one source-bounded 0-11 roll")
eq(table.concat(comply_general_story, ","),
   "get_heat_level,set_heat_level_2",
   "general compliance lowers heat once")
eq(comply_general_heat, 2,
   "general compliance lowers heat from three to two")

local comply_unarmed, comply_unarmed_pages, _, comply_unarmed_random,
      _, _, _, comply_unarmed_story, comply_unarmed_heat =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk" },
        random_values = { [12] = { 11 } },
        heat_level = 0,
        party = comply_party,
    })
eq(comply_unarmed, "resolved",
   "failed low-heat check releases a party with no listed illegal items")
eq(comply_unarmed_pages[1].text,
   "<Keeping your cool, you answer all of their questions convincingly.||" ..
   "Finally, Lone Star continues on its way.>",
   "ROM 0x234C8 no-match branch returns the release page")
eq(table.concat(comply_unarmed_random, ","), "bounded_12",
   "low-heat item scan follows the general check without another draw")
eq(table.concat(comply_unarmed_story, ","),
   "get_heat_level,set_heat_level_0",
   "no-illegal-item release clamps heat at zero")
eq(comply_unarmed_heat, 0,
   "no-illegal-item release preserves zero heat")

local special_pass_bounds = { 10, 8, 8, 2, 2, 0, 0, 2, 2 }
for heat = 0, 8 do
    local special_bound = special_pass_bounds[heat + 1]
    local special_party = {
        [0] = member(true, 2, 255),
        [1] = member(false, 0, 0),
        [2] = member(false, 0, 0),
    }
    local special_result, special_pages, _, special_random =
        run_company_man_confrontation("talk", {
            event_id = "lone_star_patrol_stop",
            task_name = "present_lone_star_patrol_stop",
            prompt_choices = { "talk" },
            random_values = {
                [special_bound] = { 0 },
                [12] = { 0 },
            },
            heat_level = heat,
            mafia_affiliation = true,
            party = special_party,
        })
    eq(special_result, "resolved",
       "Mafia affiliation remains a live Comply input at heat " .. heat)
    eq(special_pages[1].text,
       "<Keeping your cool, you answer all of their questions convincingly.||" ..
       "Finally, Lone Star continues on its way.>",
       "special-pass roll below its ROM threshold takes the general check")
    eq(table.concat(special_random, ","),
       "bounded_" .. special_bound .. ",bounded_12",
       "ROM 0x2345E source draw order at heat " .. heat)
end

local illegal_party = {
    [0] = member(true, 2, 2),
    [1] = member(false, 0, 0),
    [2] = member(false, 0, 0),
}
illegal_party[0].equipment = { { id = "ak_97" } }
local permit_result, permit_pages, _, permit_random,
      permit_titles = run_company_man_confrontation("talk", {
    event_id = "lone_star_patrol_stop",
    task_name = "present_lone_star_patrol_stop",
    prompt_choices = { "talk", "talk" },
    random_values = { [12] = { 7 } },
    party = illegal_party,
    weapon_permit = true,
})
eq(permit_result, "resolved",
   "illegal firearm with an active permit resolves without a fine")
eq(permit_titles[2],
   "Hey, Flick.  I think this guy's packing.||" ..
   "You got a permit for that illegal weapon?",
   "ROM 0x23542 displays the permit prompt after the named equipment scan")
eq(permit_pages[1].text,
   "<You show them your permit, and they continue on their patrol.>",
   "ROM 0x2354A accepts the weapon permit")
eq(table.concat(permit_random, ","), "bounded_12",
   "accepted permit avoids the payment roll")

for _, item_id in ipairs({
        "ak_97", "hk227_s", "mach_22", "roomsweeper", "frag",
        "maglock_passkey", "light_combat_armor", "heavy_combat_armor",
    }) do
    for target_slot = 0, 2 do
        local party = {
            [0] = member(true, 2, 2),
            [1] = member(target_slot >= 1, 2, 2),
            [2] = member(target_slot >= 2, 2, 2),
        }
        party[target_slot].equipment = { { id = item_id } }
        local result, pages, _, random_calls, titles =
            run_company_man_confrontation("talk", {
                event_id = "lone_star_patrol_stop",
                task_name = "present_lone_star_patrol_stop",
                prompt_choices = { "talk", "talk" },
                random_values = { [12] = { 11 } },
                party = party,
                weapon_permit = true,
            })
        local case_name = item_id .. " in party slot " .. target_slot
        eq(result, "resolved",
           case_name .. " is detected by the active-record scan")
        eq(titles[2],
           "Hey, Flick.  I think this guy's packing.||" ..
           "You got a permit for that illegal weapon?",
           case_name .. " opens the ROM weapon-permit prompt")
        eq(pages[1].text,
           "<You show them your permit, and they continue on their patrol.>",
           case_name .. " accepts the permit")
        eq(table.concat(random_calls, ","), "bounded_12",
           case_name .. " checks inventory after the general roll")
    end
end

local gap_party = {
    [0] = member(true, 2, 2),
    [1] = member(false, 0, 0),
    [2] = member(false, 0, 0),
}
gap_party[1].equipment = { { id = "frag" } }
local gap_result, gap_pages, _, gap_random, _, _, _, gap_story =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk" },
        random_values = { [12] = { 11 } },
        party = gap_party,
    })
eq(gap_result, "resolved",
   "inventory scan ends at the first inactive runner record")
eq(#gap_pages, 1,
   "inactive-slot item does not open the weapon-permit prompt")
eq(table.concat(gap_random, ","), "bounded_12",
   "inactive-slot stop adds no payment roll")
eq(table.concat(gap_story, ","),
   "get_heat_level,set_heat_level_0",
   "no active-slot match releases and lowers heat")

local bribe_party = {
    [0] = member(true, 2, 8),
    [1] = member(false, 0, 0),
    [2] = member(false, 0, 0),
}
bribe_party[0].equipment = { { id = "hk227_s" } }
local bribe_result, bribe_pages, _, bribe_random, _, _, _, bribe_story,
      bribe_heat, bribe_funds = run_company_man_confrontation("talk", {
    event_id = "lone_star_patrol_stop",
    task_name = "present_lone_star_patrol_stop",
    prompt_choices = { "talk", "talk" },
    random_values = { [12] = { 11 }, [10] = { 8 } },
    heat_level = 1,
    nuyen = 700,
    party = bribe_party,
})
eq(bribe_result, "resolved",
   "permitless armed runner can pass the 500-nuyen check")
eq(bribe_pages[1].text,
   "<You admit you must have misplaced the document.||In haste, you offer " ..
   "an \"on sight fine\" of 500 nuyen to take care of any further " ..
   "problems.||They accept your donation and leave.>",
   "ROM 0x2354A uses the source on-sight payment text")
eq(table.concat(bribe_random, ","), "bounded_12,bounded_10",
   "weapon permit route rolls the fine only after finding illegal equipment")
eq(bribe_funds, 200, "permitless route deducts exactly 500 nuyen")
eq(bribe_heat, 0, "paid permitless route lowers heat once")
eq(table.concat(bribe_story, ","),
   "get_heat_level,set_heat_level_0",
   "paid permitless route updates heat after the fine")

local suspect_party = {
    [0] = member(true, 2, 2),
    [1] = member(false, 0, 0),
    [2] = member(false, 0, 0),
}
local suspect_result, suspect_pages, _, suspect_random, suspect_titles,
      _, _, suspect_story, suspect_heat =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk", "talk" },
        random_values = { [12] = { 6 }, [20] = { 2 } },
        heat_level = 3,
        party = suspect_party,
    })
eq(suspect_result, "resolved", "suspect identification can release the party")
eq(suspect_titles[2],
   "You look an awful lot like a suspect we've been tracking.||" ..
   "Goes by the name of Joshua.  Got any ID?",
   "ROM 0x234B4 opens the suspect-ID prompt at heat three")
eq(suspect_pages[1].text,
   "<Keeping your cool, you answer all of their questions convincingly.||" ..
   "Finally, Lone Star continues on its way.>",
   "successful suspect-ID roll returns to the release record")
eq(table.concat(suspect_random, ","), "bounded_12,bounded_20",
   "suspect-ID Talk uses a 0-19 roll after the general check")
eq(table.concat(suspect_story, ","),
   "get_heat_level,set_heat_level_2",
   "successful ID check lowers heat once")
eq(suspect_heat, 2, "successful ID check lowers heat from three to two")

local suspect_failure = run_company_man_confrontation("talk", {
    event_id = "lone_star_patrol_stop",
    task_name = "present_lone_star_patrol_stop",
    prompt_choices = { "talk", "talk" },
    random_values = { [12] = { 11 }, [20] = { 3 } },
    heat_level = 3,
    party = suspect_party,
})
eq(type(suspect_failure), "table",
   "failed suspect-ID check reaches the explicit arrest-settlement boundary")
eq(suspect_failure.type, "source_boundary",
   "failed suspect-ID check does not fabricate arrest consequences")
eq(suspect_failure.reason:find("0x235AA", 1, true) ~= nil, true,
   "failed suspect-ID boundary names ROM settlement 0x235AA")

local bribe_failure, _, _, bribe_failure_random, _, _, _, bribe_failure_story,
      bribe_failure_heat, bribe_failure_funds =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk", "talk" },
        random_values = { [12] = { 11 }, [10] = { 9 } },
        heat_level = 0,
        nuyen = 700,
        party = bribe_party,
    })
eq(type(bribe_failure), "table",
   "failed on-sight fine check reaches arrest settlement")
eq(bribe_failure.type, "source_boundary",
   "failed fine check remains an explicit source boundary")
eq(bribe_failure.reason:find("0x235AA", 1, true) ~= nil, true,
   "failed fine check identifies ROM arrest settlement 0x235AA")
eq(table.concat(bribe_failure_random, ","), "bounded_12,bounded_10",
   "failed on-sight fine uses its d10 roll after the weapon scan")
eq(bribe_failure_funds, 700,
   "failed on-sight fine leaves the wallet unchanged")
eq(table.concat(bribe_failure_story, ","), "get_heat_level",
   "failed on-sight fine does not lower heat before settlement")
eq(bribe_failure_heat, 0,
   "failed on-sight fine leaves heat unchanged before settlement")

local insufficient_bribe, _, _, insufficient_bribe_random,
      _, _, _, insufficient_bribe_story, insufficient_bribe_heat,
      insufficient_bribe_funds =
    run_company_man_confrontation("talk", {
        event_id = "lone_star_patrol_stop",
        task_name = "present_lone_star_patrol_stop",
        prompt_choices = { "talk", "talk" },
        random_values = { [12] = { 11 }, [10] = { 8 } },
        heat_level = 1,
        nuyen = 499,
        party = bribe_party,
    })
eq(type(insufficient_bribe), "table",
   "successful fine roll with insufficient cash reaches arrest settlement")
eq(insufficient_bribe.type, "source_boundary",
   "insufficient funds remain an explicit source boundary")
eq(insufficient_bribe.reason:find("0x235AA", 1, true) ~= nil, true,
   "insufficient funds identify ROM arrest settlement 0x235AA")
eq(table.concat(insufficient_bribe_random, ","), "bounded_12,bounded_10",
   "insufficient-funds branch consumes the source d10 roll")
eq(insufficient_bribe_funds, 499,
   "insufficient-funds branch preserves all nuyen")
eq(table.concat(insufficient_bribe_story, ","), "get_heat_level",
   "insufficient-funds branch leaves heat unchanged")
eq(insufficient_bribe_heat, 1,
   "insufficient-funds branch preserves heat before settlement")

local leave_patrol_attack, leave_patrol_attack_pages, _, _, _, _, _,
      leave_patrol_attack_calls, leave_patrol_attack_heat =
    run_company_man_confrontation("leave", {
        random_values = { [100] = { 80 } },
        prompt_choices = { "leave", "attack" },
        heat_level = 4,
    })
eq(leave_patrol_attack, "resolved",
   "Walk Away 80 routes nested patrol Attack through 0x23412")
eq(leave_patrol_attack_pages[1].text,
   "<The Lone Star forces draw their weapons with well-trained efficiency.>",
   "nested Walk Away patrol Attack uses the shared result record")
eq(leave_patrol_attack_heat, 6,
   "nested Walk Away patrol Attack applies the shared heat increment")
eq(table.concat(leave_patrol_attack_calls, ","),
   "get_heat_level,queue_world_effect_3_3_lone_star_patrol_attack,set_heat_level_6",
   "nested Walk Away patrol Attack uses the shared queue and heat path")
end

-- ROM 0x210B4's prompt and each visible response have their own source path.
do
local company_route = DialogueFramework.describe(
    E.DialogueSource.RANDOM_ENCOUNTER,
    "field_random_events.present_company_man_toothpick")
eq(company_route.scene_outcome, E.FieldLocationSceneOutcome.SAME_SCENE,
   "Company Man returns to the source field scene")
eq(company_route.exit_behavior, E.DialogueExitBehavior.STAY,
   "Company Man uses the shared stay exit")
eq(company_route.transition, E.DialogueTransition.FADE,
   "Company Man returns through the shared fade")
eq(company_route.transition_frames,
   E.DialogueTransitionFrames.HAGGARD_BLACK_INTERVAL,
   "Company Man uses the measured field-event fade interval")
eq(company_route.input_lock,
   E.DialogueInputLock.WORLD_ACTIVITY_FFE11C,
   "Company Man preserves the resource-event input lock")
eq(company_route.input_lock_frames,
   E.DialogueInputLockFrames.WORLD_ACTIVITY,
   "Company Man preserves the resource-event lock duration")

local attack_zero, attack_zero_pages, attack_zero_exit,
      attack_zero_alarms, attack_zero_karma, attack_zero_random,
      attack_zero_titles, attack_zero_options, attack_zero_speakers =
    run_company_man_toothpick("attack", {
        -- Average Quickness is 3. Each face is 5, so fixed target 7 fails;
        -- the source target must not inherit Hidden Camera's party target.
        random_values = { [6] = { 4, 4, 4 } },
    })
eq(attack_zero, "resolved", "Company Man Attack zero-success result")
eq(attack_zero_titles[1],
   "<A Company Man walks up and flicks a toothpick at you.>",
   "Company Man prompt uses extracted opening")
eq(#attack_zero_options[1], 3, "Company Man prompt has three choices")
eq(attack_zero_options[1][1].id, "attack",
   "Company Man first choice is Attack")
eq(attack_zero_options[1][1].label, "<Attack!!>",
   "Company Man Attack label matches the source row")
eq(attack_zero_options[1][2].id, "insult",
   "Company Man second choice is Insult")
eq(attack_zero_options[1][2].label, "Slot off, frag-face!!!",
   "Company Man Insult label matches the source row")
eq(attack_zero_options[1][3].id, "leave",
   "Company Man third choice is Walk away")
eq(attack_zero_options[1][3].label, "<Walk away.>",
   "Company Man Walk away label matches the source row")
eq(attack_zero_speakers[1].speaker_id, "company_man_random_event",
   "Company Man prompt uses its source portrait binding")
eq(attack_zero_speakers[1].text_profile, "field.world_event",
   "Company Man prompt uses the field event text profile")
eq(#attack_zero_pages, 1, "failed Attack has one result page")
eq(attack_zero_pages[1].text,
   "<As you pull your guns, the Company Man flees and raises the alarm.>",
   "zero-success Attack result text")
eq(attack_zero_pages[1].continuation, false,
   "zero-success Attack result is terminal")
eq(attack_zero_pages[1].speaker_id, "company_man_random_event",
   "Attack result keeps the Company Man portrait binding")
eq(attack_zero_alarms, 1, "zero-success Attack raises the alarm")
eq(attack_zero_karma, 0, "zero-success Attack awards no Karma")
eq(#attack_zero_random, 3,
   "Attack averages active Quickness and excludes the inactive slot")
eq(table.concat(attack_zero_exit, ","), "restore_field_context_stay",
   "Company Man returns through the shared same-map dialogue exit")

local attack_one, attack_one_pages, _, attack_one_alarms, attack_one_karma,
      attack_one_random = run_company_man_toothpick("attack", {
    random_values = { [6] = { 5, 0, 4, 4 } },
})
eq(attack_one, "resolved", "Company Man Attack one-success result")
eq(attack_one_pages[1].text,
   "<You wound him as he escapes down the hall.  " ..
   "The alarm soon comes to life..>", "one-success Attack result text")
eq(attack_one_alarms, 1, "one-success Attack raises the alarm")
eq(attack_one_karma, 0, "one-success Attack awards no Karma")
eq(#attack_one_random, 4,
   "one-success Attack uses a fixed target of seven")

local attack_kill, attack_kill_pages, _, attack_kill_alarms,
      attack_kill_karma, attack_kill_random =
    run_company_man_toothpick("attack", {
        random_values = { [6] = { 5, 0, 5, 0, 4 } },
    })
eq(attack_kill, "resolved", "Company Man Attack kill result")
eq(attack_kill_pages[1].text,
   "<He does the staccato death-dance to the beat of your gun.||" ..
   "He'll not mock you again!>", "two-success Attack result text")
eq(attack_kill_alarms, 0, "successful Attack does not raise the alarm")
eq(attack_kill_karma, 1,
   "successful Attack awards one Karma to the full party")
eq(#attack_kill_random, 5, "successful Attack rolls the active average pool")

local insult_zero, insult_zero_pages, _, insult_zero_alarms,
      insult_zero_karma, insult_zero_random, _, _, insult_zero_speakers =
    run_company_man_toothpick("insult", {
        party = { [0] = member(true, 2, 2),
                 [1] = member(true, 4, 9) },
        random_values = { [6] = { 4, 4 } },
    })
eq(insult_zero, "resolved", "Company Man Insult zero-success result")
eq(insult_zero_pages[1].text,
   "<He stops and looks you over carefully.  " ..
   "He then calls security.>", "zero-success Insult result text")
eq(insult_zero_alarms, 1, "zero-success Insult raises the alarm")
eq(insult_zero_karma, 0, "zero-success Insult awards no Karma")
eq(#insult_zero_random, 2,
   "zero-success Insult uses slot 0 Charisma only")
eq(insult_zero_speakers[1].speaker_id, "company_man_random_event",
   "Insult uses the Company Man portrait binding")

local insult_one, insult_one_pages, _, insult_one_alarms,
      insult_one_karma, insult_one_random =
    run_company_man_toothpick("insult", {
        -- Slot 0 has one die; the active partner's six Charisma dice do not
        -- enter the ROM 0x2489A check.
        party = { [0] = member(true, 2, 1),
                 [1] = member(true, 4, 6) },
        random_values = { [6] = { 5, 0 } },
    })
eq(insult_one, "resolved", "Company Man Insult one-success result")
eq(insult_one_pages[1].text, "<He laughs as he walks away.>",
   "one-success Insult result text")
eq(insult_one_alarms, 0, "one-success Insult does not raise the alarm")
eq(insult_one_karma, 0, "one-success Insult awards no Karma")
eq(#insult_one_random, 2, "one-success Insult rolls first-runner Charisma")

local floor_clues = {
    { "first", "first" }, { "second", "second" },
    { "third", "third" }, { "fourth", "fourth" },
    { "fifth", "fifth" }, { "sixth", "sixth" },
}
for _, variant in ipairs(floor_clues) do
    local insult_many, insult_many_pages, insult_many_exit,
          insult_many_alarms, insult_many_karma, insult_many_random,
          _, _, _, insult_many_variant_calls =
        run_company_man_toothpick("insult", {
            party = { [0] = member(true, 2, 3) },
            random_values = { [6] = { 5, 0, 5, 0, 4 } },
            resource_variant = variant[1],
        })
    eq(insult_many, "resolved",
       "multi-success Insult resolves for resource variant " .. variant[1])
    eq(insult_many_pages[1].text,
       "<He stops and stares you down. He then smiles.>||" ..
       "He says to check the " .. variant[2] .. " floor.>",
       "Insult clue follows the selected resource variant " .. variant[1])
    eq(insult_many_pages[1].continuation, false,
       "shared 0x11CD92 result is terminal")
    eq(insult_many_pages[1].speaker_id, "company_man_random_event",
       "floor clue keeps the Company Man portrait binding")
    eq(insult_many_alarms, 0, "multi-success Insult does not raise alarm")
    eq(insult_many_karma, 0, "multi-success Insult awards no Karma")
    eq(#insult_many_random, 5,
       "multi-success Insult rolls first-runner Charisma")
    eq(insult_many_variant_calls, 1,
       "multi-success Insult reads one named resource variant")
    eq(table.concat(insult_many_exit, ","),
       "restore_field_context_stay",
       "multi-success Insult uses shared field-context restoration")
end

local insult_missing_variant, insult_missing_pages =
    run_company_man_toothpick("insult", {
        party = { [0] = member(true, 2, 3) },
        random_values = { [6] = { 5, 0, 5, 0, 4 } },
        resource_variant_unavailable = true,
    })
eq(type(insult_missing_variant), "table",
   "missing Company Man resource variant remains a source boundary")
eq(insult_missing_variant.type, "source_boundary",
   "missing resource variant is not silently clamped")
eq(#insult_missing_pages, 0,
   "missing resource variant does not invent a floor clue")

local company_leave, company_leave_pages, company_leave_exit,
      company_leave_alarms, company_leave_karma, company_leave_random =
    run_company_man_toothpick("leave")
eq(company_leave, "resolved", "Company Man Walk away result")
eq(company_leave_pages[1].text, "<He saunters away.>",
   "Walk away result matches ROM record 0x121538")
eq(company_leave_alarms, 0, "Walk away does not raise the alarm")
eq(company_leave_karma, 0, "Walk away awards no Karma")
eq(#company_leave_random, 0, "Walk away does not roll dice")
eq(table.concat(company_leave_exit, ","), "restore_field_context_stay",
   "Walk away restores the original field context")
end

-- ROM prompt 0x120F00's rows and the following text records are exercised in
-- source order; the MORE-bearing pages continue before their next record.
do
local watcher_insult, watcher_insult_pages, watcher_insult_exit,
      watcher_insult_funds, watcher_insult_random, watcher_insult_titles,
      watcher_insult_options, watcher_insult_speakers,
      watcher_insult_effects, watcher_insult_writes =
    run_watcher("insult", { body = 2 })
eq(watcher_insult, "resolved", "watcher insult resolves")
eq(watcher_insult_titles[1],
   "<The shadowy image of a watcher, a phantasmal magical messenger, " ..
   "materializes in front of you.>||Come quickly!||My master needs help!",
   "watcher prompt uses the extracted opening")
eq(#watcher_insult_options[1], 2, "watcher prompt has two choices")
eq(watcher_insult_options[1][1].id, "accept_watcher_request",
   "watcher first choice is Accept")
eq(watcher_insult_options[1][1].label, "Take me to him.  I'll help.",
   "watcher Accept label matches the source row")
eq(watcher_insult_options[1][2].id, "insult",
   "watcher second choice is Insult")
eq(watcher_insult_options[1][2].label, "Slot off, frag-face!!!",
   "watcher Insult label matches the source row")
eq(watcher_insult_speakers[1].speaker_id, "haggard_man_event",
   "watcher prompt uses the field event illustration binding")
eq(watcher_insult_speakers[1].text_profile, "field.world_event",
   "watcher prompt uses the field event text profile")
eq(#watcher_insult_pages, 1, "Insult has one terminal text page")
-- ROM text 0x11D893.
eq(watcher_insult_pages[1].text,
   "This is horrible!  Where's your compassion?!||" ..
   "<The watcher vanishes.>", "watcher Insult text")
eq(watcher_insult_pages[1].continuation, false,
   "Insult record has no continuation")
eq(watcher_insult_pages[1].speaker_id, "haggard_man_event",
   "watcher Insult keeps the event illustration binding")
eq(watcher_insult_pages[1].text_profile, "field.world_event",
   "watcher Insult keeps the event text profile")
eq(watcher_insult_funds, 125, "Insult does not award nuyen")
eq(watcher_insult_writes, 0, "Insult does not write nuyen")
eq(#watcher_insult_random, 0, "Insult does not roll Body")
eq(#watcher_insult_effects, 0, "Insult queues no actor list")
eq(table.concat(watcher_insult_exit, ","), "restore_field_context_stay",
   "watcher returns through the haggard same-map dialogue exit")

local watcher_zero, watcher_zero_pages, _, watcher_zero_funds,
      watcher_zero_random, _, _, watcher_zero_speakers,
      watcher_zero_effects, watcher_zero_writes = run_watcher(
    "accept_watcher_request", {
        body = 1,
        random_values = { [6] = { 0 } },
    })
eq(watcher_zero, "resolved", "zero-success watcher sequence resolves")
eq(#watcher_zero_pages, 3,
   "zero-success path shows accept, gang attack, and thanks pages")
-- ROM text 0x11D8D9, followed by the 0x9A Body dispatch.
eq(watcher_zero_pages[1].text,
   "<The watcher leads you through some side streets.||" ..
   "He points ahead, and you see an old man being brutally beaten " ..
   "by a group of Eye-Fivers.>", "watcher acceptance text")
eq(watcher_zero_pages[1].continuation, true,
   "acceptance record has the shared MORE label")
-- ROM text 0x11D965, which continues to record 0x120F5C.
eq(watcher_zero_pages[2].text,
   "<The mage looks up.||" ..
   "The Eye-Fivers turn around, following his gaze.>||" ..
   "Look, some new meat to tenderize!  Get him, boys!",
   "watcher zero-success text")
eq(watcher_zero_pages[2].continuation, true,
   "zero-success record has the shared MORE label")
-- ROM text 0x11DA7E.
eq(watcher_zero_pages[3].text,
   "<As the gangers come at you, you catch the mage giving you a wave " ..
   "of thanks.||He then shimmers and fades to nothingness.>",
   "watcher thanks-and-fade text")
eq(watcher_zero_pages[3].continuation, false,
   "watcher thanks-and-fade record is terminal")
eq(watcher_zero_pages[1].speaker_id, "haggard_man_event",
   "watcher acceptance keeps the field event illustration binding")
eq(watcher_zero_pages[2].text_profile, "field.world_event",
   "watcher attack result keeps the field event text profile")
eq(watcher_zero_pages[3].speaker_id, "haggard_man_event",
   "watcher thanks keeps the field event illustration binding")
eq(watcher_zero_funds, 125, "zero-success combat path does not award nuyen")
eq(watcher_zero_writes, 0, "zero-success combat path does not write nuyen")
eq(table.concat(watcher_zero_random, ","), "bounded_6",
   "zero-success branch rolls the first runner Body once")
eq(watcher_zero_effects[1],
   "queue_world_effect_5_4_phantasmal_watcher_eye_fivers_four",
   "zero-success branch queues both Eye Fiver 2 and both Eye Fiver 1")
eq(#watcher_zero_effects, 1,
   "zero-success branch queues one watcher actor list")

local watcher_one, watcher_one_pages, _, watcher_one_funds,
      watcher_one_random, _, _, _, watcher_one_effects, watcher_one_writes =
    run_watcher("accept_watcher_request", {
        body = 1,
        random_values = { [6] = { 5, 0 } },
    })
eq(watcher_one, "resolved", "one-success watcher sequence resolves")
eq(#watcher_one_pages, 3,
   "one-success path shows accept, gang attack, and thanks pages")
-- ROM text 0x11D9DE.
eq(watcher_one_pages[2].text,
   "<The mage looks at you pleadingly.||" ..
   "The Eye-Fivers turn and size you up.||" ..
   "A couple of them run away, but the rest stay behind.>||" ..
   "You want to play, too, do you?", "watcher one-success text")
eq(watcher_one_pages[2].continuation, true,
   "one-success record has the shared MORE label")
eq(watcher_one_funds, 125, "one-success combat path does not award nuyen")
eq(watcher_one_writes, 0, "one-success combat path does not write nuyen")
eq(table.concat(watcher_one_random, ","), "bounded_6,bounded_6",
   "one-success branch uses the controlled exploding-six Body roll")
eq(watcher_one_effects[1],
   "queue_world_effect_5_2_phantasmal_watcher_eye_fivers_two",
   "one-success branch queues two Eye Fiver 1 actors")
eq(#watcher_one_effects, 1,
   "one-success branch queues one watcher actor list")

local watcher_two_plus, watcher_two_pages, _, watcher_two_funds,
      watcher_two_random, _, _, _, watcher_two_effects, watcher_two_writes =
    run_watcher("accept_watcher_request", {
        body = 2,
        initial_nuyen = 375,
        random_values = { [6] = { 5, 0, 5, 0 } },
    })
eq(watcher_two_plus, "resolved", "two-success watcher reward resolves")
eq(#watcher_two_pages, 2,
   "two-success path shows accept and reward pages only")
-- ROM text 0x11DAF8.
eq(watcher_two_pages[2].text,
   "<The Eye-Fivers see you and scatter immediately.||" ..
   "The mage addresses you.>||Thank you for your help.||" ..
   "This, I'm afraid, is all I can offer in return.||" ..
   "<He hands you 200 nuyen.>", "watcher reward text")
eq(watcher_two_pages[2].continuation, false,
   "watcher reward record is terminal")
eq(watcher_two_funds, 575,
   "two-success branch adds exactly 200 nuyen")
eq(watcher_two_writes, 1,
   "two-success branch writes the reward exactly once")
eq(#watcher_two_random, 4,
   "two-success branch rolls exactly two controlled exploding sixes")
eq(#watcher_two_effects, 0,
   "two-success branch starts no Eye-Fiver sequence")

local watcher_failed_start, watcher_failed_pages, _, watcher_failed_funds,
      watcher_failed_random, _, _, _, watcher_failed_effects,
      watcher_failed_writes = run_watcher("accept_watcher_request", {
    body = 1,
    initial_nuyen = 40,
    random_values = { [6] = { 0 } },
    queue_result = "busy",
})
eq(watcher_failed_start, "resolved",
   "failed watcher list start falls through to the reward")
eq(#watcher_failed_pages, 2,
   "failed list start shows accept and reward pages")
eq(watcher_failed_funds, 240,
   "failed watcher list start adds exactly 200 nuyen")
eq(watcher_failed_writes, 1,
   "failed watcher list start writes the reward exactly once")
eq(table.concat(watcher_failed_random, ","), "bounded_6",
   "failed list start follows the source zero-success Body result")
eq(#watcher_failed_effects, 1,
   "failed list start follows one attempted actor-list queue")
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
eq(table.concat(haggard_exit, ","), "restore_field_context_stay",
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
    eq(table.concat(threaten_exit, ","), "restore_field_context_stay",
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
eq(table.concat(attack_reward_exit, ","), "restore_field_context_stay",
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
eq(table.concat(zero_success_exit, ","), "restore_field_context_stay",
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
eq(table.concat(one_success_exit, ","), "restore_field_context_stay",
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

local missing_nuyen_result = run_haggard("help", nil, {
    intelligence = 0,
    missing_nuyen = true,
    random_values = { [6] = {}, [100] = { 0 } },
})
eq(type(missing_nuyen_result), "table",
   "missing haggard reward funds returns an explicit source boundary")
eq(missing_nuyen_result.type, "source_boundary",
   "missing haggard reward funds does not call an unresolved global")

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
