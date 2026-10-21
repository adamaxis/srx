package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")
local Dialogue = require("dialogue")
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

local function new_game(choices, flags, funds)
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

    game = {
        state = state,
        funds = funds or 1000,
        location = {
            type = "building",
            id = "boris_greenhouse_frontage",
        },
        field = { event_code = 6, alternate_result = 0 },
        seen_pages = {},
        seen_events = {},
        services = {},
        ui = {},
        modules = {
            world_content_data = {
                entities = {
                    boris_greenhouse_frontage = {
                        links = { area_link, service_link },
                    },
                },
                areas = {
                    [2] = {
                        { entity_id = "boris_greenhouse_frontage" },
                    },
                },
            },
        },
    }
    game.ui.prompt_text_key = function(menu_id, text_key, choice_ids)
        game.seen_pages[#game.seen_pages + 1] = {
            menu_id = menu_id,
            text_key = text_key,
            choice_ids = choice_ids,
        }
        local choice = table.remove(choices, 1)
        assert(choice ~= nil, "unexpected Boris prompt: " .. text_key)
        for _, id in ipairs(choice_ids) do
            if id == choice then return choice end
        end
        error("choice " .. choice .. " is not available in " .. text_key)
    end
    game.ui.say = function(event_id)
        game.seen_events[#game.seen_events + 1] = event_id
        return true
    end
    game.services.call = function(service, operation)
        assert(service == "building")
        local amount = tonumber(operation:match("^charge_(%d+)$"))
        assert(amount ~= nil, "unexpected Boris payment operation")
        if game.funds < amount then return "insufficient" end
        game.funds = game.funds - amount
        return "paid"
    end
    return game
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
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.seen_pages[1].text_key == "boris.greenhouse.page.default" and
       state.seen_pages[2].text_key == "boris.greenhouse.page.opening" and
       state.seen_pages[2].choice_ids[1] == "story" and
       state.seen_pages[2].choice_ids[2] == "information" and
       state.seen_pages[2].choice_ids[3] == "later")

state = new_game({ "more", "later" }, {
    field_dialogue_flag_bit_3 = true,
})
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.state.field_dialogue_flag_bit_3 == false and
       state.seen_pages[2].text_key == "boris.greenhouse.page.repeat" and
       state.seen_pages[2].menu_id == "repeat_opening")

state = new_game({ "more", "later" }, {
    field_dialogue_flag_bit_1 = true,
})
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.state.field_dialogue_flag_bit_1 == false and
       state.seen_pages[1].text_key == "boris.greenhouse.page.alternate_0" and
       state.seen_pages[2].text_key == "boris.greenhouse.page.after_michael")

state = new_game({})
state.field.alternate_result = 1
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.seen_events[1] == "package_request")

state = new_game({ "more" })
state.field.alternate_result = -1
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.seen_events[1] == "package_payment" and
       state.seen_pages[1].text_key == "boris.greenhouse.page.followup")

state = new_game({ "more", "story", "accept", "more" }, {}, 1000)
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.funds == 900 and state.state.field_dialogue_flag_bit_0 == true and
       state.seen_pages[3].text_key == "boris.greenhouse.page.story_prompt" and
       state.seen_pages[4].text_key == "boris.greenhouse.page.story_answer")

state = new_game({ "more", "story", "accept" }, {}, 99)
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.funds == 99 and state.state.field_dialogue_flag_bit_0 == false and
       state.seen_events[1] == "insufficient_funds")

state = new_game({ "more", "information", "accept", "michael", "more" }, {
    story_flag_bit_1 = true,
}, 1000)
assert(Boris.present_indexed_title_service() == "field_return")
assert(state.funds == 500 and state.state.field_dialogue_flag_bit_2 == true and
       state.state.story_flag_bit_10 == true and
       state.state.field_dialogue_flag_bit_1 == true and
       state.state.field_mode_flag_bit_6 == false and
       state.state.field_mode_flag_bit_7 == false and
       state.state.field_mode_flag_bit_20 == true and
       state.seen_pages[3].text_key ==
           "boris.greenhouse.page.information_prompt" and
       state.seen_pages[4].choice_ids[1] == "michael" and
       state.seen_pages[4].choice_ids[2] == "david" and
       state.seen_pages[4].choice_ids[3] == "later")

state = new_game({ "david", "brother" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_1 = true,
}, 1000)
assert(Boris.present_information_offer(false) ==
       "boris_strike_team_unconverted")
assert(state.state.field_mode_flag_bit_6 == false and
       state.state.field_mode_flag_bit_7 == false and
       state.state.field_mode_flag_bit_20 == true and
       state.seen_pages[1].choice_ids[2] == "david" and
       state.seen_pages[2].text_key ==
           "boris.greenhouse.page.david_information")

state = new_game({ "ilene", "later" }, {
    field_dialogue_flag_bit_2 = true,
    story_flag_bit_18 = true,
}, 1000)
assert(Boris.present_information_offer(false) == "field_return")
assert(state.state.field_mode_effect_flag_bit_0 == true and
       state.seen_pages[1].choice_ids[2] == "ilene")

local event = Dialogue.event_text_keys(
    "boris_greenhouse_narrator", "boris.greenhouse", "package_request")
assert(event and
       event.title_key == "boris.greenhouse.page.alternate_1" and
       event.continuation_key == "")
local menu = Dialogue.menu_text_keys(
    "boris_greenhouse_narrator", "boris.greenhouse", "story_payment",
    { "accept", "later" }, true)
assert(menu and #menu.choice_label_keys == 2 and
       menu.choice_label_keys[1] == "boris.greenhouse.option.accept" and
       menu.choice_label_keys[2] == "boris.greenhouse.option.decline")

print("Boris Greenhouse tests passed: Scene 2/event 6 same-map service and Lua dialogue branches")
