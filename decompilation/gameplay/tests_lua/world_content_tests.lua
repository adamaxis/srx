package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local World = require("world_content")
local E = require("enums")

game = {
    modules = {
        world_content_data = {
            world_activity_actors = {
                {
                    id = "field_activity_scene_13_ilene_gate_script_actor_0",
                    scene = 13,
                    activity_id = "scene_13_ilene_gate_script",
                    activity_kind = "scene_script",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
                    actor_name = "lone11",
                    visual_profile_id = E.ActorVisualProfile.LONE_STAR_MAGE,
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x1100,
                    world_y = 0x0300,
                    initial_heading = 0xC000,
                    action_kind = "item",
                    action_id = "american_l36",
                    ordered_overrides = "0x2E=0xC0,0x9E=0x10",
                    actor_facts = { roles = {}, interaction = {} },
                },
                {
                    id = "field_activity_scene_47_strike_team_actor_0",
                    scene = 47,
                    activity_id = "scene_47_strike_team",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "unspecified",
                    pending_world_effect_requirement = "unspecified",
                    actor_name = "Strike Team",
                },
                {
                    id = "field_activity_scene_20_ilene_activity_actor_0",
                    scene = 20,
                    activity_id = "scene_20_ilene_activity",
                    activity_kind = "event_driven",
                    world_effect_active_requirement = "inactive",
                    pending_world_effect_requirement = "pending",
                    actor_name = "lone11",
                    visual_profile_id = E.ActorVisualProfile.LONE_STAR_MAGE,
                    profile_id = "lone11_k0a_item03",
                    palette_line = 1,
                    sprite_stem = "actor-kind-10-palette-1",
                    world_x = 0x0E80,
                    world_y = 0x0700,
                    initial_heading = 0x8000,
                    action_kind = "item",
                    action_id = "american_l36",
                    ordered_overrides = "0x9E=0x20,0x2E=0x80,0x9E=0x8",
                    actor_facts = { roles = {}, interaction = {} },
                },
            },
        },
    },
}

local actors = World.world_activity_actors_for_activity_id(
    "scene_13_ilene_gate_script", 13)
assert(#actors == 1, "selector lookup returned the wrong actor count")
assert(actors[1].id ==
       "field_activity_scene_13_ilene_gate_script_actor_0" and
       actors[1].name == "lone11" and
       actors[1].visual_profile_id == E.ActorVisualProfile.LONE_STAR_MAGE and
       actors[1].palette_line == 1 and
       actors[1].sprite == "actor-kind-10-palette-1" and
       actors[1].x == 0x1100 and actors[1].y == 0x0300 and
       actors[1].heading == 0xC000 and
       actors[1].action_id == "american_l36" and
       actors[1].activity_kind == "scene_script" and
       actors[1].ordered_overrides == nil and
       actors[1].actor_facts.roles ~= nil,
       "selector lookup lost the typed host actor facts")
local pending = World.world_activity_actors_for_activity_id(
    "scene_20_ilene_activity", 20)
assert(#pending == 1 and
       pending[1].id ==
           "field_activity_scene_20_ilene_activity_actor_0" and
       pending[1].activity_kind == "event_driven" and
       pending[1].name == "lone11" and
       pending[1].visual_profile_id == E.ActorVisualProfile.LONE_STAR_MAGE and
       pending[1].x == 0x0E80 and pending[1].y == 0x0700 and
       pending[1].heading == 0x8000 and
       pending[1].ordered_overrides == nil and
       pending[1].actor_facts.roles ~= nil,
       "activity-id lookup did not preserve the pending Ilene actor")
assert(#World.world_activity_actors_for_activity_id(
           "scene_47_strike_team", 20) == 0,
       "activity-id lookup crossed scene boundaries")
assert(#World.world_activity_actors_for_activity_id(
           "scene_13_ilene_gate_script") == 1,
       "semantic activity lookup unexpectedly requires a scene number")
assert(World.pending_world_activity_id_for_scene(20, false, true) ==
           "scene_20_ilene_activity" and
       World.pending_world_activity_id_for_scene(20, true, true) ==
           nil and
       World.pending_world_activity_id_for_scene(20, false, false) ==
           nil and
       World.pending_world_activity_id_for_scene(20, true, false) ==
           nil and
       World.pending_world_activity_id_for_scene(47, false, true) ==
           nil,
       "pending actor-list selector ignored its extracted marker gates")
assert(#World.world_activity_actors_for_activity_id(
           "scene_13_ilene_gate_script", "13") == 0,
       "activity lookup accepted a nonnumeric scene")

-- ROM 0x1DED8 scans live field actor records and requests the 0x1E514 escort
-- cleanup path [C]. Lua selects actor state through its semantic live flag.
local DialogueActions = require("dialogue_actions")
local E = require("enums")
local escorted_actor = {
    live_field_actor = true,
    actor_facts = {
        lifecycle = { active = true },
        behavior = { escort = true },
    },
}
local slot_only_actor = {
    source_record_slot = 7,
    live_field_actor = false,
    actor_facts = {
        lifecycle = { active = true },
        behavior = { escort = true },
    },
}
game.actors = {
    ["world-escort"] = escorted_actor,
    ["legacy-slot-only"] = slot_only_actor,
}
local cleanup_registry = DialogueActions.new_default_registry()
assert(cleanup_registry:execute("actor.conclude_escort_relationship"))
assert(escorted_actor.pending_lifecycle_event ==
           E.ActorLifecycleEvent.ESCORT_CONCLUDED and
       slot_only_actor.pending_lifecycle_event == nil,
       "escort conclusion relied on a numeric record slot instead of actor facts")

-- ROM 0x17034/0x17236 is the confirmed Scene-10 default +0x9E bit-4 actor.
-- 0xECCC selects variant 4 in this route; 0x21174 uses signed FFFC12 to
-- select the 0xFE640 subrow (negative +0xC, >=4 +8, otherwise +0).
local generic_contact_action = World.field_actor_contact_task(
    E.Scene.FRIENDSHIP_RESTAURANT,
    "field_activity_scene_10_default_script_actor_0",
    "scene_10_default_script", E.FieldActorContactHandler.ORDINARY)
assert(generic_contact_action ==
           E.FieldActorContactAction.GENERIC_DIALOGUE,
       "ROM-confirmed Scene-10 bit-4 contact did not select generic dialogue")
assert(World.generic_field_contact_row_offset(-1) ==
           E.GenericFieldContact.ROW_OFFSET_NEGATIVE and
       World.generic_field_contact_row_offset(4) ==
           E.GenericFieldContact.ROW_OFFSET_HIGH and
       World.generic_field_contact_row_offset(0) ==
           E.GenericFieldContact.ROW_OFFSET_DEFAULT,
       "generic contact row did not follow the signed FFFC12 bands")
assert(World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_default_script",
           E.GenericFieldContact.VARIANT_STANDARD) ==
           E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL,
       "Scene-10 variant 4 did not resolve its confirmed neutral row")
assert(World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_default_script",
           E.GenericFieldContact.VARIANT_ALTERNATE) == nil and
       World.generic_field_contact_dialogue(
           E.Scene.FRIENDSHIP_RESTAURANT, "scene_10_event_6_script",
           E.GenericFieldContact.VARIANT_ALTERNATE) ==
               "scene_10_group_2_default" and
       World.field_actor_contact_task(
           E.Scene.FRIENDSHIP_RESTAURANT,
           "field_activity_scene_10_default_script_actor_0",
           "scene_10_default_script",
           E.FieldActorContactHandler.WORLD_EFFECT) ==
               E.FieldActorContactAction.START_ACTIVITY,
       "generic dialogue escaped its source-confirmed actor/row/handler")
local generic_text = require("data.field_contact_dialogue_text")

assert(generic_text[E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL] ==
           "<A huge female ork dressed in AmerIndian garb is here." ..
               "||She seems occupied with some fetishes.>",
       "Scene-10 neutral text no longer matches ROM 0x101E24")
local presented_contact_text, presented_continuation
game.location = {
    type = "field_map",
    id = E.GenericFieldContact.LOCATION_ID,
    event_id = E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL,
}
game.ui = {
    say_text = function(text, continuation)
        presented_contact_text = text
        presented_continuation = continuation
    end,
}
local GenericFieldContacts = require("field_generic_contacts")
local imported_contact_rows = require("data.field_contact_dialogue_rows")
assert(GenericFieldContacts.present() == "presented" and
       presented_contact_text == generic_text[
           E.GenericFieldContactResponse.SCENE_10_DEFAULT_NEUTRAL] and
       presented_continuation == false,
       "generic contact presentation did not open the selected Lua row")

local Json = require("json")
local function load_runner_dialogue_catalog(name)
    local file = assert(io.open("decompilation/gameplay/dialogue/" .. name .. ".json", "rb"))
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end
game.content = {
    speakers = load_runner_dialogue_catalog("speakers"),
    contacts = load_runner_dialogue_catalog("contacts"),
    menus = load_runner_dialogue_catalog("menus"),
}
local Dialogue = require("dialogue")

-- The production field contact task must enter the same speaker/menu flow for
-- each hireable runner, not reduce non-Ricky contacts to narrator text.
local petr_prompt_count = 0
local petr_inline_text
local petr_speaker
game.location = {
    type = "field_map",
    id = E.GenericFieldContact.LOCATION_ID,
    event_id = "scene_14_group_3_default",
    speaker_id = "petr_uvehr",
    scene_id = 14,
    actor_instance_id = "petr_contact",
}
game.actors = {
    petr_contact = {
        scene_id = 14,
        actor_facts = { lifecycle = { active = true } },
    },
}
game.ui = {
    prompt_text = function(menu_id, text, choices, speaker_id, text_profile)
        petr_prompt_count = petr_prompt_count + 1
        petr_inline_text = text
        assert(menu_id == "opening" and #choices == 3 and
               choices[1] == "information" and choices[2] == "hire" and
               choices[3] == "leave",
               "Petr did not receive the ROM common conversation choices")
        petr_speaker = Dialogue.menu_text_keys(
            game.location.speaker_id, "runner.petr_uvehr", menu_id,
            choices, true)
        return "leave"
    end,
    say_text = function(text, continuation, speaker_id, text_profile)
        petr_inline_text = text
        petr_speaker = { speaker_id, text_profile }
    end,
}
assert(GenericFieldContacts.present() == "presented" and
       petr_prompt_count == 1 and
       petr_inline_text == imported_contact_rows.responses[
           "scene_14_group_3_default"].opening_text and
       petr_speaker ~= nil and
       Dialogue.speaker_profile("petr_uvehr").portrait_alias ==
           "field.hud.party.face.6.palette.2",
       "Petr field contact did not use its profiled common conversation")

local function run_ricky_flow(choices, subject, hire_result)
    local choice_index = 0
    local followup_count = 0
    local events = {}
    local offer_text
    local service_calls = {}
    local actor = {
        scene_id = 19,
        actor_facts = { lifecycle = { active = true } },
    }
    game.location = {
        type = "field_map",
        id = E.GenericFieldContact.LOCATION_ID,
        event_id = "scene_19_group_0_default",
        speaker_id = "ricky",
        scene_id = 19,
        actor_instance_id = "ricky_contact",
    }
    game.actors = { ricky_contact = actor }
    game.services = {
        call = function(domain, action)
            assert(domain == "recruited_runner")
            service_calls[#service_calls + 1] = action
            if action == "mark_recruited_runner_information" then return "applied" end
            if action == "prepare_recruited_runner_offer" then
                return hire_result == "roster_full"
                    and "roster_full" or "fees_100_900"
            end
            if action == "hire_recruited_runner_short_term" or
               action == "hire_recruited_runner_permanent" then
                return hire_result
            end
            error("unexpected Ricky service action: " .. action)
        end,
    }
    game.ui = {
        prompt_text = function(menu_id, text, menu_choices)
            if menu_id == "opening" then
                choice_index = choice_index + 1
                return choices[choice_index]
            end
            if menu_id == "hire_offer" then
                offer_text = text
                assert(#menu_choices == 2 and
                       menu_choices[1] == "short_term" and
                       menu_choices[2] == "permanent")
            end
            choice_index = choice_index + 1
            return choices[choice_index]
        end,
        prompt_text_key = function(menu_id, text_key, menu_choices,
                                   speaker_id, text_profile)
            local menu = Dialogue.menu_text_keys(
                speaker_id, text_profile, menu_id, menu_choices, true)
            assert(menu_id == "story_followup" and speaker_id == "ricky" and
                   text_profile == "runner.ricky" and
                   menu ~= nil and
                   menu_choices[1] == "story")
            followup_count = followup_count + 1
            events[#events + 1] = text_key:sub(#"runner.ricky." + 1)
            return followup_count == 1 and "questions" or
                "different_subject"
        end,
        menu = function(menu_id, menu_choices)
            assert(menu_id == "subjects" and #menu_choices == 3)
            return subject
        end,
        say = function(event_id, speaker_id, text_profile)
            assert(speaker_id == "ricky" and
                   text_profile == "runner.ricky")
            events[#events + 1] = event_id
        end,
        say_text = function() error("Ricky bypassed his speaker profile") end,
    }
    local result = GenericFieldContacts.present()
    return result, events, offer_text, service_calls, actor
end

local ricky_hired, ricky_events, ricky_offer, ricky_services, ricky_actor =
    run_ricky_flow({ "information", "hire", "short_term" },
                   "shaman_magic", "hired")
assert(ricky_hired == "presented" and
       ricky_events[1] == "information.opening" and
       ricky_events[2] == "information.continuation" and
       ricky_events[3] == "subject.shaman_magic.page_0" and
       ricky_events[5] == "subject.shaman_magic.page_2" and
       ricky_events[#ricky_events] == "hire.success" and
       ricky_offer:find("$100", 1, true) and
       ricky_offer:find("$900", 1, true) and
       ricky_actor.actor_facts.lifecycle.active == false and
       ricky_services[1] == "mark_recruited_runner_information" and
       ricky_services[2] == "prepare_recruited_runner_offer" and
       ricky_services[3] == "hire_recruited_runner_short_term",
       "Ricky information, live pricing, or short-term hire flow failed")

-- ROM 0xFE908 -> 0x1001AF is the third subject's Gunderson answer [C].
local ricky_johnson_result, ricky_johnson_events =
    run_ricky_flow({ "information", "leave" }, "johnson", "not_selected")
assert(ricky_johnson_result == "presented" and
       ricky_johnson_events[3] ==
           "subject.johnson_gunderson.page_0" and
       #ricky_johnson_events == 3,
       "Ricky's Johnson subject did not display its ROM response page")

local ricky_insufficient, insufficient_events, _, _, insufficient_actor =
    run_ricky_flow({ "hire", "permanent", "leave" },
                   "local_area", "insufficient_funds")
assert(ricky_insufficient == "presented" and
       insufficient_events[1] == "hire.insufficient" and
       insufficient_actor.actor_facts.lifecycle.active == true,
       "Ricky's unaffordable permanent offer changed party actor state")

local ricky_full, full_events, _, _, full_actor =
    run_ricky_flow({ "hire", "leave" }, "johnson", "roster_full")
assert(ricky_full == "presented" and
       full_events[1] == "hire.roster_full" and
       full_actor.actor_facts.lifecycle.active == true,
       "Ricky's full-roster response did not return to the common choices")



local runner_content_keys = Dialogue.runner_text_keys()
local text_file = assert(io.open("decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local hireable_runner_routes = {
    { id = "winston_marrs", scene = 11,
      response = "scene_11_group_1_default",
      portrait = "field.hud.party.face.4.palette.1" },
    { id = "trent", scene = 10,
      response = "scene_10_group_2_default",
      portrait = "field.hud.party.face.5.palette.1" },
    { id = "petr_uvehr", scene = 14, response = "scene_14_group_3_default",
      portrait = "field.hud.party.face.6.palette.2" },
    { id = "walking_bear", scene = 10, response = "scene_10_default_neutral",
      portrait = "field.hud.party.face.7.palette.2" },
    { id = "phantom", scene = 16, response = "scene_16_group_5_default",
      portrait = "field.hud.party.face.8.palette.0" },
    { id = "two_fists", scene = 13, response = "scene_13_group_6_default",
      portrait = "field.hud.party.face.9.palette.0" },
    { id = "freya", scene = 17, response = "scene_17_group_7_default",
      portrait = "field.hud.party.face.10.palette.1" },
    { id = "rianna", scene = 18, response = "scene_18_group_8_default",
      portrait = "field.hud.party.face.11.palette.0" },
}
for _, route in ipairs(hireable_runner_routes) do
    local speaker = Dialogue.speaker_profile(route.id)
    local text_profile = "runner." .. route.id
    local runner = Dialogue.runner_conversation(route.id)
    local event_log = {}
    local service_log = {}
    local actor_id = route.id .. "_contact"
    local root_count = 0
    local followup_count = 0
    local offer_text
    game.location = {
        type = "field_map",
        id = E.GenericFieldContact.LOCATION_ID,
        event_id = route.response,
        speaker_id = route.id,
        scene_id = route.scene,
        actor_instance_id = actor_id,
    }
    game.actors = { [actor_id] = {
        scene_id = route.scene,
        actor_facts = { lifecycle = { active = true } },
    } }
    game.services = {
        call = function(domain, action)
            assert(domain == "recruited_runner")
            service_log[#service_log + 1] = action
            if action == "mark_recruited_runner_information" then
                return "applied"
            elseif action == "prepare_recruited_runner_offer" then
                return "fees_1234_5678"
            elseif action == "hire_recruited_runner_short_term" then
                return "insufficient_funds"
            end
            error("unexpected runner service action: " .. action)
        end,
    }
    game.ui = {
        prompt_text = function(menu_id, text, choices)
            local menu = Dialogue.menu_text_keys(
                route.id, text_profile, menu_id, choices, true)
            assert(menu ~= nil, "runner menu profile missing: " .. route.id .. "." .. menu_id)
            if menu_id == "opening" then
                root_count = root_count + 1
                return root_count == 1 and "information" or
                    root_count == 2 and "hire" or "leave"
            elseif menu_id == "hire_offer" then
                offer_text = text
                return "short_term"
            end
            error("unexpected runner prompt: " .. menu_id)
        end,
        prompt_text_key = function(menu_id, text_key, choices,
                                   speaker_id, profile_id)
            local menu = Dialogue.menu_text_keys(
                speaker_id, profile_id, menu_id, choices, true)
            assert(menu ~= nil and menu_id == "story_followup" and
                   speaker_id == route.id and profile_id == text_profile and
                   menu.choice_label_keys[1] == "boris.greenhouse.option.story" and
                   script_text[menu.choice_label_keys[1]] ==
                       "What's your story, chummer?",
                   "runner story choice differs from its ROM menu text")
            local content_prefix = "runner." .. route.id .. "."
            assert(text_key:sub(1, #content_prefix) == content_prefix and
                   script_text[text_key] ~= nil,
                   "runner follow-up must stay on its final ROM text page")
            local event_id = text_key:sub(#content_prefix + 1)
            local event = Dialogue.event_text_keys(
                speaker_id, profile_id, event_id)
            assert(event ~= nil and event.title_key == text_key)
            event_log[#event_log + 1] = event_id
            followup_count = followup_count + 1
            return ({ "story", "questions", "different_subject" })[
                followup_count]
        end,
        menu = function(menu_id, choices)
            local menu = Dialogue.menu_text_keys(
                route.id, text_profile, menu_id, choices, false)
            assert(menu ~= nil, "runner subject menu profile missing: " .. route.id)
            return choices[1]
        end,
        say = function(event_id, speaker_id, profile_id)
            assert(speaker_id == route.id and profile_id == text_profile)
            local event = Dialogue.event_text_keys(speaker_id, profile_id, event_id)
            assert(event ~= nil and script_text[event.title_key] ~= nil,
                   "runner dialogue page missing its exact text profile: " ..
                       route.id .. "." .. event_id)
            event_log[#event_log + 1] = event_id
        end,
        say_text = function() error("runner contact fell back to narrator text") end,
    }
    assert(speaker ~= nil and speaker.portrait_alias == route.portrait and
           GenericFieldContacts.present() == "presented",
           "hireable runner did not complete its profiled field conversation: " .. route.id)
    local biography = {}
    for _, page_id in ipairs({ "opening", "continuation", "final" }) do
        if runner.information[page_id] ~= nil then
            biography[#biography + 1] = "information." .. page_id
        end
    end
    local topic = runner.subject.option_order[1]
    local response_pages = runner.subject.responses[topic]
    local prefix = "runner." .. route.id .. "."
    local expected_events = {}
    for _, page in ipairs(biography) do expected_events[#expected_events + 1] = page end
    for _, page in ipairs(biography) do expected_events[#expected_events + 1] = page end
    for _, page in ipairs(response_pages) do
        expected_events[#expected_events + 1] = page:sub(#prefix + 1)
    end
    expected_events[#expected_events + 1] = "hire.insufficient"
    assert(#event_log == #expected_events,
           "runner dialogue page count differs from ROM route: " .. route.id)
    for index, event_id in ipairs(expected_events) do
        assert(event_log[index] == event_id,
               "runner dialogue page order differs from ROM route: " .. route.id)
    end
    assert(offer_text:find("$1234", 1, true) and
           offer_text:find("$5678", 1, true) and
           service_log[1] == "mark_recruited_runner_information" and
           service_log[2] == "prepare_recruited_runner_offer" and
           service_log[3] == "hire_recruited_runner_short_term" and
           followup_count == 3,
           "runner hire pricing, refusal, or story re-offer failed: " .. route.id)
end

local all_runner_subject_routes = {
    { id = "ricky", scene = 19, response = "scene_19_group_0_default",
      portrait = "field.hud.party.face.3.palette.0" },
}
for _, route in ipairs(hireable_runner_routes) do
    all_runner_subject_routes[#all_runner_subject_routes + 1] = route
end
local function verify_runner_subject_route(route, topic_id)
    local runner = Dialogue.runner_conversation(route.id)
    local speaker = Dialogue.speaker_profile(route.id)
    local text_profile = "runner." .. route.id
    local actor_id = route.id .. "_subject_route"
    local event_log = {}
    local service_log = {}
    local root_count = 0
    local followup_count = 0
    local followups = { "story", "questions", "different_subject" }
    local prefix = "runner." .. route.id .. "."
    game.location = {
        type = "field_map",
        id = E.GenericFieldContact.LOCATION_ID,
        event_id = route.response,
        speaker_id = route.id,
        scene_id = route.scene,
        actor_instance_id = actor_id,
    }
    game.actors = { [actor_id] = {
        scene_id = route.scene,
        actor_facts = { lifecycle = { active = true } },
    } }
    game.services = {
        call = function(domain, action)
            assert(domain == "recruited_runner")
            service_log[#service_log + 1] = action
            if action == "mark_recruited_runner_information" or
               (route.id == "trent" and topic_id == "contacts" and
                action == "record_trent_jefferson_contact") then
                return "applied"
            end
            error("unexpected runner subject service action: " .. action)
        end,
    }
    game.ui = {
        prompt_text = function(menu_id, _, choices)
            assert(menu_id == "opening" and choices[1] == "information" and
                   choices[2] == "hire" and choices[3] == "leave")
            root_count = root_count + 1
            return root_count == 1 and "information" or "leave"
        end,
        prompt_text_key = function(menu_id, text_key, choices,
                                   speaker_id, profile_id)
            local menu = Dialogue.menu_text_keys(
                speaker_id, profile_id, menu_id, choices, true)
            assert(menu_id == "story_followup" and speaker_id == route.id and
                   profile_id == text_profile and menu ~= nil and
                   menu.choice_label_keys[1] ==
                       "boris.greenhouse.option.story" and
                   choices[1] == "story" and
                   script_text["boris.greenhouse.option.story"] ==
                       "What's your story, chummer?",
                   "runner subject page lost the ROM story follow-up")
            assert(text_key:sub(1, #prefix) == prefix and
                   script_text[text_key] ~= nil,
                   "runner follow-up references missing script text")
            local event_id = text_key:sub(#prefix + 1)
            local event = Dialogue.event_text_keys(
                speaker_id, profile_id, event_id)
            assert(event ~= nil and event.title_key == text_key,
                   "runner subject page is missing from its speaker profile")
            event_log[#event_log + 1] = event_id
            followup_count = followup_count + 1
            return followups[followup_count]
        end,
        menu = function(menu_id, choices)
            local menu = Dialogue.menu_text_keys(
                route.id, text_profile, menu_id, choices, false)
            assert(menu_id == "subjects" and menu ~= nil and
                   #choices == #runner.subject.option_order,
                   "runner subject choice list differs from its profile")
            for index, id in ipairs(runner.subject.option_order) do
                assert(choices[index] == id and
                       runner.subject.options[id] ~= nil and
                       runner.subject.responses[id] ~= nil and
                       menu.choice_label_keys[index] ==
                           runner.subject.options[id],
                       "runner subject menu ID has no matching label/response: " ..
                           route.id .. "." .. id)
            end
            return topic_id
        end,
        say = function(event_id, speaker_id, profile_id)
            local event = Dialogue.event_text_keys(
                speaker_id, profile_id, event_id)
            assert(speaker_id == route.id and profile_id == text_profile and
                   event ~= nil and script_text[event.title_key] ~= nil,
                   "runner page missing from the selected speaker profile")
            event_log[#event_log + 1] = event_id
        end,
        say_text = function()
            error("runner subject route fell back to narrator text")
        end,
    }

    assert(speaker ~= nil and runner ~= nil and
           speaker.portrait_alias == route.portrait and
           GenericFieldContacts.present() == "presented",
           "runner subject route did not return to the common choices: " ..
               route.id .. "." .. topic_id)

    local biography = {}
    for _, page_id in ipairs({ "opening", "continuation", "final" }) do
        if runner.information[page_id] ~= nil then
            biography[#biography + 1] = "information." .. page_id
        end
    end
    local expected_events = {}
    for _ = 1, 2 do
        for _, page_id in ipairs(biography) do
            expected_events[#expected_events + 1] = page_id
        end
    end
    for _, page in ipairs(runner.subject.responses[topic_id]) do
        expected_events[#expected_events + 1] = page:sub(#prefix + 1)
    end
    assert(#event_log == #expected_events and followup_count == 3,
           "runner subject page or story follow-up count differs: " ..
               route.id .. "." .. topic_id)
    local expected_service_count =
        route.id == "trent" and topic_id == "contacts" and 2 or 1
    assert(#service_log == expected_service_count and
           service_log[1] == "mark_recruited_runner_information" and
           ((expected_service_count == 2 and
             service_log[2] == "record_trent_jefferson_contact") or
            expected_service_count == 1),
           "runner subject did not apply its source selection action once: " ..
               route.id .. "." .. topic_id)
    for index, event_id in ipairs(expected_events) do
        assert(event_log[index] == event_id,
               "runner selected a different subject response page: " ..
                   route.id .. "." .. topic_id)
    end
end

for _, route in ipairs(all_runner_subject_routes) do
    local runner = Dialogue.runner_conversation(route.id)
    assert(runner ~= nil and runner.subject ~= nil and
           #runner.subject.option_order > 0,
           "hireable runner has no ordered subject menu: " .. route.id)
    for _, topic_id in ipairs(runner.subject.option_order) do
        verify_runner_subject_route(route, topic_id)
    end
end

local expected_runner_content = {
    { content_id = "petr_uvehr.subject.title", text_key = "runner.petr_uvehr.subject.title", text = "Like what?" },
    { content_id = "petr_uvehr.subject.option.matrix", text_key = "runner.petr_uvehr.subject.option.matrix", text = "Tell me about the Matrix." },
    { content_id = "petr_uvehr.subject.option.dwarf", text_key = "runner.petr_uvehr.subject.option.dwarf", text = "How is it that you are a dwarf?" },
    { content_id = "petr_uvehr.subject.matrix.page_0", text_key = "runner.petr_uvehr.subject.matrix.page_0", text = "The Matrix is an area in cyberspace that basically connects all computer systems together.|Any system that is hooked into the Matrix can be accessed.|Once inside a system, we can take datafiles and sell them to people called fixers." },
    { content_id = "petr_uvehr.subject.matrix.page_1", text_key = "runner.petr_uvehr.subject.matrix.page_1", text = "Deckers like me would rather be in cyberspace than in this dreary world any day." },
    { content_id = "petr_uvehr.subject.dwarf.page_0", text_key = "runner.petr_uvehr.subject.dwarf.page_0", text = "<He grimaces a bit, but starts to talk.>|We've been called a lot of things, from mutations to freaks.|We prefer metahumans. " },
    { content_id = "petr_uvehr.subject.dwarf.page_1", text_key = "runner.petr_uvehr.subject.dwarf.page_1", text = "Dwarfs are one of the four metahuman groups, the others being elves, trolls, and orks.|As you can see, we are generally stouter and stronger than you, but those are about the major differences." },
    { content_id = "walking_bear.information.final", text_key = "runner.walking_bear.information.final", text = "To help them, I needed credits quick.|I offered my services of healing, as well as my more forceful disciplines." },
    { content_id = "walking_bear.subject.title", text_key = "runner.walking_bear.subject.title", text = "Like what?" },
    { content_id = "walking_bear.subject.option.shaman_magic", text_key = "runner.walking_bear.subject.option.shaman_magic", text = "Tell me about shaman magic." },
    { content_id = "walking_bear.subject.option.david_owlfeather", text_key = "runner.walking_bear.subject.option.david_owlfeather", text = "Do you know where I can find David Owlfeather?" },
    { content_id = "walking_bear.subject.option.ork", text_key = "runner.walking_bear.subject.option.ork", text = "How is it that you are an ork?" },
    { content_id = "walking_bear.subject.shaman_magic.page_0", text_key = "runner.walking_bear.subject.shaman_magic.page_0", text = "Bear is mighty.|He struggles against no one, and looks after his own.|Shaman who follow Bear must listen to all of the earth's creatures and aid them when possible.|Bear also will not allow injustice and will strike when necessary." },
    { content_id = "walking_bear.subject.david_owlfeather.page_0", text_key = "runner.walking_bear.subject.david_owlfeather.page_0", text = "Owlfeather is a good man.|Have you searched the Salish-Shidhe?" },
    { content_id = "walking_bear.subject.ork.page_0", text_key = "runner.walking_bear.subject.ork.page_0", text = "<She looks into your eyes.>|Many orks shame me, for they make us all seem savage.|Orks are naturally larger than humans, so you fear us, and we react.|But that is no excuse for their behavior." },
    { content_id = "phantom.information.final", text_key = "runner.phantom.information.final", text = "Before I knew it, I was out.  No pension, no severence, just a pink slip.|But I showed them. I just took my share from their system accounts.|But I'm ready to do some serious damage to those high-minded breeders.|No offense, of course." },
    { content_id = "phantom.subject.title", text_key = "runner.phantom.subject.title", text = "What do you want?" },
    { content_id = "phantom.subject.option.matrix", text_key = "runner.phantom.subject.option.matrix", text = "Tell me about the Matrix." },
    { content_id = "phantom.subject.option.cyberdeck_equipment", text_key = "runner.phantom.subject.option.cyberdeck_equipment", text = "Where can I get some cyberdeck equipment?" },
    { content_id = "phantom.subject.option.elf", text_key = "runner.phantom.subject.option.elf", text = "How is it that you are an elf?" },
    { content_id = "phantom.subject.matrix.page_0", text_key = "runner.phantom.subject.matrix.page_0", text = "Most people go after the datastores because that's where the quick nuyen is.|But the systems are much more than datastore dumps.|The systems have vast connections, great influence." },
    { content_id = "phantom.subject.matrix.page_1", text_key = "runner.phantom.subject.matrix.page_1", text = "Security cameras, alerts, and elevators can be controlled inside the system.|If you wanna go somewhere restricted, go into the Matrix, jack into their system, and find the Slave Nodes.|Or better yet, crash the system CPU." },
    { content_id = "phantom.subject.cyberdeck_equipment.page_0", text_key = "runner.phantom.subject.cyberdeck_equipment.page_0", text = "There are a few cyberdeck stores around.|There's one called Microtronics here in the Arcology.  They sell decent decks.|The other's in the Puyallup Barrens, inside the Crime Mall, but the stuff they sell is mostly hot and costs a bundle." },
    { content_id = "phantom.subject.elf.page_0", text_key = "runner.phantom.subject.elf.page_0", text = "They called it the \"Goblinization.\"|It happened about thirty years ago when one in ten humans suddenly metamorphosed into two of the metahuman races, orks and trolls.|Elves and dwarves were born of human parents about forty years ago when the magic itself first returned." },
    { content_id = "phantom.subject.elf.page_1", text_key = "runner.phantom.subject.elf.page_1", text = "The elves were one of the races that created their own nations, and grouped into tribes.|They created a tight-knit family, one that excluded many outsiders.|But most of that you can find in the history books." },
    { content_id = "two_fists.information.final", text_key = "runner.two_fists.information.final", text = "I'll always be grateful to you for pulling me outta that hole, Joshua." },
    { content_id = "two_fists.subject.title", text_key = "runner.two_fists.subject.title", text = "Regarding what?" },
    { content_id = "two_fists.subject.option.thon", text_key = "runner.two_fists.subject.option.thon", text = "What do you know about someone named Thon?" },
    { content_id = "two_fists.subject.option.ellisia", text_key = "runner.two_fists.subject.option.ellisia", text = "What can you tell me about Ellisia?" },
    { content_id = "two_fists.subject.thon.page_0", text_key = "runner.two_fists.subject.thon.page_0", text = "Word inside was he's new to Seattle and he came here looking for something, but nobody knows what. He was rumored to be working with Renraku on something, maybe related." },
    { content_id = "two_fists.subject.thon.page_1", text_key = "runner.two_fists.subject.thon.page_1", text = "I also heard one of Thon's lieutenants is named Vigore. He did some time in Hollywood, but I stayed out of his way.  " },
    { content_id = "two_fists.subject.ellisia.page_0", text_key = "runner.two_fists.subject.ellisia.page_0", text = "He was a hero to my people. His deeds are legend, but they are best spoken of by someone more gifted than I in his lore. |The Elders believe his name still carries power, but most of the young doubt it." },
    { content_id = "freya.information.final", text_key = "runner.freya.information.final", text = "But I grew tired of their control over me and escaped into the shadows.|Been running them ever since." },
    { content_id = "freya.subject.title", text_key = "runner.freya.subject.title", text = "What's on your mind, chummer?" },
    { content_id = "freya.subject.option.vigore", text_key = "runner.freya.subject.option.vigore", text = "What do you know about a Johnson named Vigore?" },
    { content_id = "freya.subject.option.local_johnsons", text_key = "runner.freya.subject.option.local_johnsons", text = "Do you know of any Johnsons in the area?" },
    { content_id = "freya.subject.vigore.page_0", text_key = "runner.freya.subject.vigore.page_0", text = "He's too slick for his own good.|The contracts are good and there is no doubt that the nuyen is good, but there's more going on with him than meets the eye.|Be careful of that one." },
    { content_id = "freya.subject.local_johnsons.page_0", text_key = "runner.freya.subject.local_johnsons.page_0", text = "Well, there's Julius Strouther, who I like to do runs for.|He's got a booth reserved over at Matchsticks in Downtown Seattle.|He's as close to an honest dwarf as you'll find in this sprawl; fair with your contracts and your pay." },
    { content_id = "rianna.information.final", text_key = "runner.rianna.information.final", text = "Without deckers like myself, the Corps would grow unchecked.|They would have even more power than they do now, which is saying a lot." },
    { content_id = "rianna.subject.title", text_key = "runner.rianna.subject.title", text = "Like...?" },
    { content_id = "rianna.subject.option.matrix", text_key = "runner.rianna.subject.option.matrix", text = "Tell me about the Matrix." },
    { content_id = "rianna.subject.option.corporate_security", text_key = "runner.rianna.subject.option.corporate_security", text = "Tell me about Corporate security procedures." },
    { content_id = "rianna.subject.matrix.page_0", text_key = "runner.rianna.subject.matrix.page_0", text = "The Matrix holds some keen realities.|I've heard stories some Corps are actually creating life in the Matrix.|Artificial Intelligences.|It's all possible in the Matrix." },
    { content_id = "rianna.subject.matrix.page_1", text_key = "runner.rianna.subject.matrix.page_1", text = "Even so, my talents and skills will never be duplicated by some program.|If you want to crack a system, nothing can compare to raw elven talent.|If you want to run a Corp, I'm your woman." },
    { content_id = "rianna.subject.corporate_security.page_0", text_key = "runner.rianna.subject.corporate_security.page_0", text = "Simple.  They are deadly.|If you don't have top level software in an awesome deck, not even my skills could get you in." },
    { content_id = "rianna.subject.corporate_security.page_1", text_key = "runner.rianna.subject.corporate_security.page_1", text = "If you got the wares, the best way to run a Corp system is by jacking in from inside the Corp.|From there, you can enter the system via I/O ports, which are usually less protected then the SANs.|Much easier to get what you're after." },
}
for _, expected in ipairs(expected_runner_content) do
    assert(runner_content_keys[expected.content_id] == expected.text_key,
        "runner content ID did not resolve: " .. expected.content_id)
    assert(script_text[expected.text_key] == expected.text,
        "runner text differs from the ROM record: " .. expected.text_key)
end

local expected_runner_biographies = {
    ["runner.petr_uvehr.information.opening"] = "I came up here to drown my sorrows.  But I guess you want to talk.|I'm called Petr.  I worked in the tech division of the Stuffer Shack where I learned my way around a cyberdeck.",
    ["runner.petr_uvehr.information.continuation"] = "I was jealous of those cyber-cowboys, with their guns and chrome.|The Matrix got them all they wanted, so I stole me a deck, plugged in, and I've never looked back!",
    ["runner.walking_bear.information.opening"] = "I am Walking Bear.  I was born in the city and was happy until I changed into the form you see now.|Then no one would talk to me and many made fun of me.|I ran into the wilderness and never wanted to return.",
    ["runner.walking_bear.information.continuation"] = "There I met some AmerIndian tribes.  They were nice and taught me the way of the shaman.|Bear reached out to me because he knew I needed his calm words, and he knew I could take care of my AmerIndian brothers.",
    ["runner.walking_bear.information.final"] = "To help them, I needed credits quick.|I offered my services of healing, as well as my more forceful disciplines.",
    ["runner.phantom.information.opening"] = "I've been shooting the Matrix for a bit.|My handle is Phantom.|Heard of me?",
    ["runner.phantom.information.continuation"] = "I've been making a name for myself in the Matrix.|I worked for a company who trained me in cyberspace.|Then one of those anti-metahuman groups showed up, screaming about purity and them losing jobs and garbage like that.",
    ["runner.phantom.information.final"] = "Before I knew it, I was out.  No pension, no severence, just a pink slip.|But I showed them. I just took my share from their system accounts.|But I'm ready to do some serious damage to those high-minded breeders.|No offense, of course.",
    ["runner.two_fists.information.opening"] = "I went to the Barrens to find some action. At the time I thought my life needed some adventure.|Anyway, I get involved with these joyboys who tell me they know how to get their hands on some serious nuyen.",
    ["runner.two_fists.information.continuation"] = "So I hook up with them on this run against Ares, and what do you know?  My buddies set off the alarm while I was checking ahead. Before I knew what hit me, Corp Strike Teams were swarming all over and I end up in Hollywood Pen.",
    ["runner.two_fists.information.final"] = "I'll always be grateful to you for pulling me outta that hole, Joshua.",
    ["runner.freya.information.opening"] = "I hear you're the new golden child.  Number one with a bullet, and all that jazz.  So, I guess I can trust you.  I'm Freya.|When I was young, I discovered I had latent magical skills.",
    ["runner.freya.information.continuation"] = "My tribe, the Cascade Crow, viewed hermetic mages as blasphemous, so I ran away and fell in with a radical political group.|They trained me to become an assassin, using magic as my weapon, with Corporate officials as my targets.|I did my job, no sweat.",
    ["runner.freya.information.final"] = "But I grew tired of their control over me and escaped into the shadows.|Been running them ever since.",
    ["runner.rianna.information.opening"] = "Heartbane was an ancient elven leader.|She prophesied the Matrix, \"a place where everything and nothing exists.|A place where wars are fought and blood is shed without weapons\".|She knew it was going to be big!",
    ["runner.rianna.information.continuation"] = "So I use Heartbane as my street name, coupled with my true name, Rianna.|I've been running the Corps since the inception of the Matrix.|I've seen just about every type of IC and flatlined my share of decker cowboys.",
    ["runner.rianna.information.final"] = "Without deckers like myself, the Corps would grow unchecked.|They would have even more power than they do now, which is saying a lot.",
}
for text_key, expected in pairs(expected_runner_biographies) do
    assert(script_text[text_key] == expected,
           "runner biography differs from the ROM record: " .. text_key)
end
assert(script_text["runner.ricky.hire.insufficient_funds"] ==
           "You don't seem to have the right stuff, chum.||Nuyen, that is.||Come see me when that credstick starts to burn a hole in your armor." and
       script_text["runner.ricky.hire.success"] ==
           "You got me, pal.||No regrets, you'll see." and
       script_text["runner.ricky.hire.roster_full"] ==
           "Looks like you already got your hands full, pal.||Come on back when you can free up a spot for my talents.",
       "runner hire/refusal pages differ from the shared ROM records")

print("world content tests passed")
