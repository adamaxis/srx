package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("route_test_support").wrap(require("building"))
local Owlfeather = require("salish_owlfeather")
local RedBuffalo = require("salish_red_buffalo")
local Json = require("json")
local function load_gameplay_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local source = file:read("*a")
    file:close()
    return Json.decode(source)
end
local script_text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(script_text_file:read("*a")).texts
script_text_file:close()

local choices = {}
local seen_menus = {}
local services = {}
local dialogues = {}
local function select_choice(menu_id, text, options)
    seen_menus[#seen_menus + 1] = { menu_id = menu_id, text = text, choices = options }
    local selected = table.remove(choices, 1)
    assert(selected ~= nil, "missing test choice for " .. menu_id)
    for _, option in ipairs(options) do
        if option == selected then return selected end
    end
    error("invalid test choice " .. selected .. " for " .. menu_id)
end

local state_facts = {}
local condition_definitions = {}
game = {
    state = setmetatable({}, {
        __index = function(_, name) return state_facts[name] end,
        __newindex = function(_, name, value) state_facts[name] = value end,
    }),
    field = { package_run_status = "none" },
    location = { type = "building", id = "salish_owlfeather_ranger_lodge" },
    modules = { world_content_data = { entities = {}, areas = {} } },
    content = {
        speakers = load_gameplay_json("dialogue/speakers"),
        menus = load_gameplay_json("dialogue/menus"),
        conditions = load_gameplay_json("dialogue/conditions"),
        cinematics = load_gameplay_json("dialogue/cinematics"),
        conversations = {
            harlequin_main_conversation = load_gameplay_json(
                "dialogue/conversations/harlequin_main_conversation"),
        },
        location_conditions = {
            schemaVersion = 1, conditions = condition_definitions,
        },
    },
    ui = {
        prompt_text = select_choice,
        prompt_text_key = function(menu_id, text_key, options,
                                   speaker_id, text_profile)
            local menu_profile = game.content.menus.profiles[text_profile]
            local menu = menu_profile and menu_profile[menu_id]
            assert(menu ~= nil and script_text[text_key] ~= nil and
                   speaker_id == game.location.speaker_id and
                   text_profile == game.location.text_profile,
                "Harlequin prompt is missing its active location presentation")
            for _, choice in ipairs(options) do
                assert(menu.choice_label_keys[choice] ~= nil and
                       script_text[menu.choice_label_keys[choice]] ~= nil,
                    "Harlequin prompt has no catalog label for " .. choice)
            end
            return select_choice(menu_id, script_text[text_key], options)
        end,
        menu = function(menu_id, options, speaker_id, text_profile)
            local menu_profile = game.content.menus.profiles[text_profile]
            local menu = menu_profile and menu_profile[menu_id]
            assert(menu ~= nil and speaker_id == game.location.speaker_id and
                   text_profile == game.location.text_profile,
                "Harlequin menu is missing its active location presentation")
            for _, choice in ipairs(options) do
                assert(menu.choice_label_keys[choice] ~= nil and
                       script_text[menu.choice_label_keys[choice]] ~= nil,
                    "Harlequin menu has no catalog label for " .. choice)
            end
            return select_choice(menu_id, nil, options)
        end,
        say = function(event_id, speaker_id, text_profile,
                       presentation, input_delay_frames)
            local profile = game.content.speakers.profiles[speaker_id]
            local event = profile and profile.events[event_id]
            assert(event ~= nil and profile.text_profile == text_profile and
                   speaker_id == game.location.speaker_id and
                   text_profile == game.location.text_profile,
                "Harlequin page is missing its active location presentation")
            local text = script_text[event.title_key]
            assert(text ~= nil,
                "Harlequin page has no generated text: " .. event.title_key)
            dialogues[#dialogues + 1] = {
                text = text,
                event_id = event_id,
                speaker_id = speaker_id,
                text_profile = text_profile,
                presentation = presentation,
                input_delay_frames = input_delay_frames,
            }
            return true
        end,
        say_text = function(text, continuation)
            dialogues[#dialogues + 1] = { text = text, continuation = continuation }
        end,
    },
    services = {
        call = function(domain, action)
            services[#services + 1] = { domain, action }
            return "applied"
        end,
    },
}

local function equal(fact, value)
    return { fact = fact, op = "eq", value = value }
end
local function condition(id, clauses)
    condition_definitions[id] = { all = clauses }
    return id
end

local function bind_entity(scene, entity_id, links)
    game.modules.world_content_data.entities = {
        [entity_id] = { links = links },
    }
    game.modules.world_content_data.areas = {
        [scene] = { { entity_id = entity_id } },
    }
end

local function route(scene, event_code, event_index, event_table)
    return Building.resolve_field_location_route({
        scene_id = scene,
        building_interaction_mode = false,
        event_code = event_code,
        event_index = event_index,
        event_table_available = true,
        event_table = event_table,
        package_run_status = "none",
    })
end

local owl_links = {
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "area_event", script = "area_event_table",
        destination_scene = 13, destination_mode = "field", route_priority = 0,
    },
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 30,
        route_condition_id = condition("test.owlfeather.first", {
            equal("story.tarislar_asked_shale", true),
            equal("story.owlfeather_asked", false),
        }),
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_first_contact",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 20,
        route_condition_id = condition("test.owlfeather.search", {
            equal("story.owlfeather_asked", true),
            equal("story.aragorn_found", false),
        }),
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_search_followup",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 10,
        route_condition_id = condition("test.owlfeather.return", {
            equal("story.aragorn_found", true),
            equal("story.aragorn_freed_told", false),
        }),
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_return_followup",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
}
bind_entity(6, "salish_owlfeather_ranger_lodge", owl_links)
local event_table = { 13 }
game.state.tarislar_asked_shale = true
game.state.owlfeather_asked = false
local selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_first_contact")

game.state.tarislar_asked_shale = false
game.state.owlfeather_asked = true
game.state.aragorn_found = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_search_followup")

game.state.owlfeather_asked = false
game.state.aragorn_found = true
game.state.aragorn_freed_told = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_return_followup")

game.state.aragorn_found = false
game.state.aragorn_freed_told = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "area_event" and selected.area_event_target == 13)

-- The JSON graph dispatches these reusable typed actions. Its actual text,
-- conditions, and branch permutations are exercised through GameScriptRuntime.
local StoryFacts = require("story_facts")
local DialogueActions = require("dialogue_actions")
local E = require("enums")
local action_registry = DialogueActions.new_default_registry()
game.state.owlfeather_asked = false
local succeeded, result = action_registry:execute(
    "party.award_karma", { amount = 1 })
assert(succeeded and result == "applied")
succeeded, result = action_registry:execute(
    "story.set_fact", { factId = E.StoryFact.OWLFEATHER_ASKED, value = true })
assert(succeeded and result == nil and game.state.owlfeather_asked == true)

game.state.aragorn_freed_told = false
succeeded, result = action_registry:execute(
    "party.award_karma", { amount = 2 })
assert(succeeded and result == "applied")
succeeded, result = action_registry:execute(
    "story.set_fact", { factId = E.StoryFact.ARAGORN_FREED_TOLD, value = true })
assert(succeeded and result == nil and game.state.aragorn_freed_told == true)

succeeded, result = action_registry:execute(
    "story.apply_dialogue_command", { commandId = 0xFF })
assert(not succeeded and result == nil)

local red_links = {
    {
        source_scene = 6, source_event = "event_1",
        route_kind = "area_event", script = "area_event_table",
        destination_scene = 13, destination_mode = "field", route_priority = 0,
    },
    {
        source_scene = 6, source_event = "event_1",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 10,
        indexed_title_service_module = "salish_red_buffalo",
        indexed_title_service_function = "present_indexed_title_service",
        location_type = "building", location_id = "salish_red_buffalo_lodge",
        location_text_profile = "salish.red_buffalo", location_speaker_id = "red_buffalo_woman",
    },
}
bind_entity(6, "salish_red_buffalo_lodge", red_links)
selected = route(6, "event_1", 1, { 0, 13 })
local red_buffalo_service = selected.indexed_title_service
assert(selected.kind == "field_service" and
       red_buffalo_service.module == "salish_red_buffalo" and
       red_buffalo_service.location_type == "building" and
       red_buffalo_service.location_id == "salish_red_buffalo_lodge" and
       red_buffalo_service.text_profile == "salish.red_buffalo" and
       red_buffalo_service.speaker_id == "red_buffalo_woman")
-- Production sets the active location context before invoking the service.
-- This unit test calls the Lua service directly, so install the exact context
-- returned by its selected world-content task first.
game.location.type = red_buffalo_service.location_type
game.location.id = red_buffalo_service.location_id
game.location.text_profile = red_buffalo_service.text_profile
game.location.speaker_id = red_buffalo_service.speaker_id

game.state.ritual_of_binding = true
game.state.red_buffalo_woman = false
game.state.notebook_tip_flag_bit_5 = false
game.state.notebook_tip_flag_bit_6 = true
game.state.field_mode_effect_flag_bit_29 = true
game.state.field_mode_effect_flag_bit_30 = false
choices = { "more", "more" }
assert(RedBuffalo.present_indexed_title_service() == "field_return")
assert(game.state.red_buffalo_woman == true)
assert(game.state.notebook_tip_flag_bit_5 == true and
       game.state.notebook_tip_flag_bit_6 == false and
       game.state.field_mode_effect_flag_bit_29 == false and
       game.state.field_mode_effect_flag_bit_30 == true)

-- ROM prompt node 0xF5888 has separate no-business and More outcomes. The
-- former leaves the lodge; the latter advances into the penthouse passage.
game.state.ritual_of_binding = false
game.state.red_buffalo_woman = false
game.state.ito_fight_won = false
local dialogue_count = #dialogues
choices = { "more", "no_business" }
assert(RedBuffalo.present_indexed_title_service() == "field_return")
assert(#dialogues == dialogue_count + 1 and
       dialogues[#dialogues].text:find("quickly leave", 1, true) ~= nil and
       dialogues[#dialogues].text:find("penthouse", 1, true) == nil)

dialogue_count = #dialogues
local menu_count = #seen_menus
choices = {
    "more", "more", "more", "ask_about_michael", "who_is_thon",
    "more", "more", "why_renraku", "more",
}
assert(RedBuffalo.present_indexed_title_service() == "field_return")
assert(#dialogues == dialogue_count + 2 and
       dialogues[dialogue_count + 1].text:find("enter the penthouse", 1, true) ~= nil and
       dialogues[#dialogues].text:find("relationship with Renraku", 1, true) ~= nil and
       dialogues[#dialogues].event_id == "renraku_update" and
       dialogues[#dialogues].speaker_id == "red_buffalo_woman" and
       dialogues[#dialogues].text_profile == "salish.red_buffalo" and
       dialogues[dialogue_count + 1].text:find("quickly leave", 1, true) == nil and
       dialogues[#dialogues].text:find("quickly leave", 1, true) == nil)
assert(#seen_menus == menu_count + 9 and
       seen_menus[menu_count + 1].menu_id == "introduction" and
       seen_menus[menu_count + 2].menu_id == "no_gift" and
       seen_menus[menu_count + 3].menu_id == "meeting" and
       seen_menus[menu_count + 4].menu_id == "michael_topic" and
       seen_menus[menu_count + 5].menu_id == "thon_topic" and
       seen_menus[menu_count + 6].menu_id == "thon_explanation" and
       seen_menus[menu_count + 7].menu_id == "thon_curse" and
       seen_menus[menu_count + 8].menu_id == "renraku_topic" and
       seen_menus[menu_count + 9].menu_id == "renraku_explanation")
assert(game.state.field_mode_effect_flag_bit_7 == true)

print("Salish lodge tests passed: ROM route gates, typed actions, dialogue, and same-map return")
