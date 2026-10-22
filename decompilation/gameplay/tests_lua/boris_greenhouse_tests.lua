package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local source = file:read("*a")
    file:close()
    return Json.decode(source)
end

local script_text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(script_text_file:read("*a")).texts
script_text_file:close()

local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = {
        boris_greenhouse = load_json("dialogue/conversations/boris_greenhouse"),
    },
}

local Building = require("building")
local Boris = require("boris_greenhouse")

local area_link = {
    source_scene = 2,
    source_event = "event_6",
    route_kind = "area_event",
    script = "area_event_table",
    destination_scene = 26,
    destination_mode = "resource_activity",
    route_priority = 0,
}
local service_link = {
    source_scene = 2,
    source_event = "event_6",
    route_kind = "field_service",
    script = "lua_field_service",
    source_table_destination = 26,
    route_priority = 10,
    indexed_title_service_module = "boris_greenhouse",
    indexed_title_service_function = "present_indexed_title_service",
    location_type = "building",
    location_id = "boris_greenhouse_frontage",
    location_text_profile = "boris.greenhouse",
    location_speaker_id = "boris_greenhouse_narrator",
}

local function new_game(choices, flags, funds, alternate_result)
    local queued_choices = {}
    for index, choice in ipairs(choices or {}) do
        queued_choices[index] = choice
    end
    local state = {
        field_dialogue_flag_bit_0 = false,
        field_dialogue_flag_bit_1 = false,
        field_dialogue_flag_bit_2 = false,
        field_dialogue_flag_bit_3 = false,
        story_flag_bit_1 = false,
        story_flag_bit_3 = false,
        story_flag_bit_9 = false,
        story_flag_bit_10 = false,
        story_flag_bit_18 = false,
        story_flag_bit_19 = false,
        group_item_flag_bit_5 = false,
        field_mode_flag_bit_6 = true,
        field_mode_flag_bit_7 = true,
        field_mode_flag_bit_20 = false,
        field_mode_effect_flag_bit_0 = false,
    }
    for name, value in pairs(flags or {}) do state[name] = value end

    local instance = {
        state = state,
        funds = funds or 1000,
        location = { type = "building", id = "boris_greenhouse_frontage" },
        field = { event_code = 6, alternate_result = alternate_result or 0 },
        content = content,
        seen_pages = {},
        seen_events = {},
        service_calls = {},
        services = {},
        modules = {
            world_content_data = {
                entities = { boris_greenhouse_frontage = {
                    links = { area_link, service_link },
                } },
                areas = { [2] = { { entity_id = "boris_greenhouse_frontage" } } },
            },
        },
        ui = {},
    }

    local function take_choice(prompt_id, available_choices)
        local selected = table.remove(queued_choices, 1)
        assert(selected ~= nil, "unexpected Boris prompt: " .. prompt_id)
        for _, choice_id in ipairs(available_choices) do
            if choice_id == selected then return selected end
        end
        error("choice " .. tostring(selected) .. " is not available in " .. prompt_id ..
              " (visible: " .. table.concat(available_choices, ",") .. ")")
    end

    instance.ui.say = function(event_id, speaker_id, text_profile)
        local speaker = content.speakers.profiles[speaker_id]
        local event = speaker and speaker.events[event_id]
        assert(event ~= nil, "unknown Boris dialogue event: " .. tostring(event_id))
        local text_key = event.title_key
        assert(script_text[text_key] ~= nil,
            "Boris event has no extracted text: " .. tostring(text_key))
        instance.seen_events[#instance.seen_events + 1] = {
            id = event_id,
            text_key = text_key,
            text_profile = text_profile,
        }
        if event.continuation_key == "shared.more" then
            assert(table.remove(queued_choices, 1) == "more",
                "expected MORE after Boris event " .. event_id)
        end
        return true
    end

    instance.ui.prompt_text_key = function(menu_id, text_key, choice_ids)
        assert(script_text[text_key] ~= nil,
            "Boris prompt has no extracted text: " .. tostring(text_key))
        instance.seen_pages[#instance.seen_pages + 1] = {
            menu_id = menu_id,
            text_key = text_key,
            choice_ids = choice_ids,
        }
        return take_choice(menu_id .. ":" .. text_key, choice_ids)
    end

    instance.ui.menu = function(menu_id, choice_ids, speaker_id, text_profile)
        local profile = content.menus.profiles[text_profile]
        local menu = profile and profile[menu_id]
        assert(menu ~= nil, "unknown Boris menu: " .. tostring(menu_id))
        for _, choice_id in ipairs(choice_ids) do
            local label_key = menu.choice_label_keys[choice_id]
            assert(label_key ~= nil and script_text[label_key] ~= nil,
                "Boris menu choice has no extracted label: " .. tostring(choice_id))
        end
        instance.seen_pages[#instance.seen_pages + 1] = {
            menu_id = menu_id,
            choice_ids = choice_ids,
        }
        return take_choice(menu_id, choice_ids)
    end

    instance.services.call = function(domain, operation)
        assert(domain == "building", "unexpected Boris service: " .. tostring(domain))
        local amount = tonumber(operation:match("^charge_(%d+)$"))
        assert(amount ~= nil, "unexpected Boris payment operation: " .. tostring(operation))
        instance.service_calls[#instance.service_calls + 1] = amount
        if instance.funds < amount then return "insufficient" end
        instance.funds = instance.funds - amount
        return "paid"
    end

    game = instance
    return instance
end

local context = {
    scene_id = 2,
    building_interaction_mode = false,
    event_code = "event_6",
    event_index = 6,
    event_table_available = true,
    event_table = { 0, 0, 0, 0, 0, 0, 26 },
    alternate_result = 0,
}

local state = new_game({ "more", "later" })
local route = Building.resolve_field_location_route(context)
assert(route.kind == "field_service" and route.script == "lua_field_service")
assert(route.indexed_title_service and
       route.indexed_title_service.module == "boris_greenhouse" and
       route.indexed_title_service.function_name ==
           "present_indexed_title_service")
local outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and #state.service_calls == 0)
assert(state.seen_events[1].text_key == "boris.greenhouse.page.default" and
       state.seen_pages[1].text_key == "boris.greenhouse.page.opening" and
       state.seen_pages[1].choice_ids[1] == "story" and
       state.seen_pages[1].choice_ids[2] == "information" and
       state.seen_pages[1].choice_ids[3] == "later")

state = new_game({ "more", "more", "later" }, {
    field_dialogue_flag_bit_3 = true,
})
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.state.field_dialogue_flag_bit_3 == false and
       state.seen_events[2].text_key == "boris.greenhouse.page.repeat" and
       state.seen_pages[1].text_key == "boris.greenhouse.page.repeat")

state = new_game({ "more", "later" }, {
    field_dialogue_flag_bit_1 = true,
})
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.state.field_dialogue_flag_bit_1 == false and
       state.seen_events[1].text_key == "boris.greenhouse.page.alternate_0" and
       state.seen_pages[1].text_key == "boris.greenhouse.page.after_michael")

state = new_game({}, nil, 1000, 1)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.seen_events[1].text_key == "boris.greenhouse.page.alternate_1")

state = new_game({ "more" }, nil, 1000, -1)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.seen_events[1].text_key == "boris.greenhouse.page.alternate_2" and
       state.seen_events[2].text_key == "boris.greenhouse.page.followup")

state = new_game({ "more", "story", "accept", "more" }, {}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and state.funds == 900 and
       state.state.field_dialogue_flag_bit_0 == true and
       state.service_calls[1] == 100 and
       state.seen_pages[2].text_key == "boris.greenhouse.page.story_prompt" and
       state.seen_events[2].text_key == "boris.greenhouse.page.story_answer")

state = new_game({ "more", "story", "accept" }, {}, 99)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and state.funds == 99 and
       state.state.field_dialogue_flag_bit_0 == false and
       state.seen_events[2].text_key ==
           "boris.greenhouse.page.insufficient_funds")

state = new_game({ "more", "story", "more" }, {
    field_dialogue_flag_bit_0 = true,
})
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and #state.service_calls == 0 and
       state.seen_events[2].text_key == "boris.greenhouse.page.story_answer")

state = new_game({ "more", "information", "accept", "michael", "more" }, {
    story_flag_bit_1 = true,
}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and state.funds == 500 and
       state.state.field_dialogue_flag_bit_2 == true and
       state.state.story_flag_bit_10 == true and
       state.state.field_dialogue_flag_bit_1 == true and
       state.state.field_mode_flag_bit_6 == false and
       state.state.field_mode_flag_bit_7 == false and
       state.state.field_mode_flag_bit_20 == true and
       state.seen_pages[2].text_key == "boris.greenhouse.page.information_prompt" and
       state.seen_pages[3].text_key == "boris.greenhouse.page.paid_information" and
       state.seen_pages[3].choice_ids[1] == "michael" and
       state.seen_pages[3].choice_ids[2] == "david" and
       state.seen_pages[3].choice_ids[3] == "later")

state = new_game({ "more", "information", "david", "brother" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_1 = true,
    story_flag_bit_3 = false,
}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "source_boundary" and
       outcome.reason:find("0x21BE4", 1, true) ~= nil and
       state.state.field_mode_flag_bit_6 == false and
       state.state.field_mode_flag_bit_7 == false and
       state.state.field_mode_flag_bit_20 == true and
       state.seen_pages[2].text_key ==
           "boris.greenhouse.page.paid_information_repeat" and
       state.seen_pages[2].choice_ids[2] == "david" and
       state.seen_pages[3].text_key ==
           "boris.greenhouse.page.david_information")

state = new_game({ "more", "information", "ilene", "later" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_18 = true,
    story_flag_bit_19 = false,
    group_item_flag_bit_5 = false,
}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.state.field_mode_effect_flag_bit_0 == true and
       state.seen_pages[2].choice_ids[2] == "ilene" and
       state.seen_pages[3].text_key == "boris.greenhouse.page.ilene_information")

state = new_game({ "more", "information", "thon", "more" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_9 = true,
}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "continue_in_place" and
       state.seen_pages[2].choice_ids[2] == "thon" and
       state.seen_events[2].text_key == "boris.greenhouse.page.thon_information")

state = new_game({ "more", "information", "david", "brother" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_1 = true,
    story_flag_bit_3 = false,
    story_flag_bit_18 = true,
    story_flag_bit_19 = false,
    group_item_flag_bit_5 = false,
}, 1000)
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "source_boundary" and
       state.seen_pages[2].choice_ids[2] == "david" and
       state.seen_pages[2].choice_ids[3] ~= "ilene")

state = new_game({ "more", "information" }, { field_dialogue_flag_bit_2 = true })
state.state.group_item_flag_bit_5 = nil
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "source_boundary" and
       outcome.reason:find("condition_unknown", 1, true) ~= nil and
       #state.seen_pages == 1)

state = new_game({})
state.field.alternate_result = nil
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "source_boundary" and
       outcome.reason:find("condition_unknown", 1, true) ~= nil)

state = new_game({})
state.location.id = "another_building"
outcome = Boris.present_indexed_title_service()
assert(outcome.type == "source_boundary" and
       outcome.reason:find("precondition", 1, true) ~= nil)

print("Boris Greenhouse tests passed: JSON graph branches, ordered gates, payments, typed outcomes, and source boundaries")
