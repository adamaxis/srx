package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Building = require("building")
local Owlfeather = require("salish_owlfeather")
local RedBuffalo = require("salish_red_buffalo")

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
local active_story_trigger
game = {
    state = setmetatable({}, {
        __index = function(_, name)
            local trigger = type(name) == "string" and
                name:match("^story_trigger_active_(%d+)$")
            if trigger ~= nil then
                return active_story_trigger == tonumber(trigger)
            end
            return state_facts[name]
        end,
        __newindex = function(_, name, value)
            local trigger = type(name) == "string" and
                name:match("^story_trigger_active_(%d+)$")
            if trigger ~= nil then
                -- True models writing the FFDD64 index. Nil is used by the
                -- fixture only to reset the register between independent
                -- interactions; production Lua never clears this register.
                if value == true then
                    active_story_trigger = tonumber(trigger)
                elseif value == nil and
                       active_story_trigger == tonumber(trigger) then
                    active_story_trigger = nil
                end
                return
            end
            state_facts[name] = value
        end,
    }),
    field = { mode = 6, event_code = 0, alternate_result = 0 },
    location = { type = "building", id = "salish_owlfeather_ranger_lodge" },
    modules = { world_content_data = { entities = {}, areas = {} } },
    ui = {
        prompt_text = select_choice,
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
        alternate_result = 0,
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
        route_conditions = "story_flag_bit_1=true;story_flag_bit_2=false",
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_first_contact",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 20,
        route_conditions = "story_flag_bit_2=true;story_flag_bit_3=false",
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_search_followup",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
    {
        source_scene = 6, source_event = "event_0",
        route_kind = "field_service", script = "lua_field_service",
        route_priority = 10,
        route_conditions = "story_flag_bit_3=true;story_flag_bit_4=false",
        indexed_title_service_module = "salish_owlfeather",
        indexed_title_service_function = "present_return_followup",
        location_type = "building", location_id = "salish_owlfeather_ranger_lodge",
        location_text_profile = "salish.owlfeather", location_speaker_id = "david_owlfeather",
    },
}
bind_entity(6, "salish_owlfeather_ranger_lodge", owl_links)
local event_table = { 13 }
game.state.story_flag_bit_1 = true
game.state.story_flag_bit_2 = false
local selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_first_contact")

game.state.story_flag_bit_1 = false
game.state.story_flag_bit_2 = true
game.state.story_flag_bit_3 = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_search_followup")

game.state.story_flag_bit_2 = false
game.state.story_flag_bit_3 = true
game.state.story_flag_bit_4 = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "field_service" and
       selected.indexed_title_service.function_name == "present_return_followup")

game.state.story_flag_bit_3 = false
game.state.story_flag_bit_4 = false
selected = route(6, "event_0", 0, event_table)
assert(selected.kind == "area_event" and selected.area_event_target == 13)

-- The first service requires the Inn clue, sets trigger 2 only when the
-- player asks about Michael, and follows the ROM's two nested question pages.
game.state.story_flag_bit_1 = true
game.state.story_flag_bit_2 = false
game.state.story_flag_bit_3 = false
game.state.story_flag_bit_4 = false
game.state.field_mode_effect_flag_bit_8 = true
game.state.field_mode_effect_flag_bit_9 = true
game.field.mode = 6
game.field.event_code = 0
choices = { "brother", "more", "learned", "more", "like_who", "task", "thanks" }
assert(Owlfeather.present_first_contact() == "field_return")
assert(game.state.story_trigger_active_2 == true and
       game.state.story_flag_bit_2 == true)
assert(game.state.field_mode_effect_flag_bit_8 == false and
       game.state.field_mode_effect_flag_bit_9 == false and
       game.state.field_mode_effect_flag_bit_10 == true)
assert(#services == 1 and services[1][1] == "runner" and
       services[1][2] == "adjust_source_byte_triplet_1")
assert(seen_menus[1].menu_id == "opening" and
       seen_menus[1].choices[3] == "brother")

-- Trigger 3 is the missed-Aragorn follow-up. Its Later branch shows both
-- farewell pages and leaves the repeat-service gate open.
game.state.story_flag_bit_2 = true
game.state.story_flag_bit_3 = false
game.state.field_mode_effect_flag_bit_9 = true
choices = { "later" }
local farewell_count = #dialogues
assert(Owlfeather.present_search_followup() == "field_return")
assert(game.state.story_trigger_active_3 == true and
       game.state.story_flag_bit_3 == false and
       game.state.field_mode_effect_flag_bit_9 == false)
assert(seen_menus[#seen_menus].menu_id == "search_followup" and
       seen_menus[#seen_menus].choices[3] == "later")
assert(#dialogues == farewell_count + 2 and
       dialogues[farewell_count + 1].text:find("Sure thing", 1, true) ~= nil and
       dialogues[farewell_count + 1].continuation == true and
       dialogues[farewell_count + 2].text:find("Farewell", 1, true) ~= nil and
       dialogues[farewell_count + 2].continuation == false)

-- Trigger 4's return branch applies its D0=2 runner adjustment, effect flags,
-- command 0x19, and the handler's unconditional trigger tail.
game.state.story_flag_bit_3 = true
game.state.story_flag_bit_4 = false
game.state.story_trigger_active_2 = nil
game.state.field_mode_effect_flag_bit_8 = true
game.state.field_mode_effect_flag_bit_9 = true
game.state.field_mode_effect_flag_bit_10 = true
game.state.field_mode_effect_flag_bit_13 = true
game.state.field_mode_effect_flag_bit_14 = false
choices = { "freed", "more", "more", "more", "more", "thanks" }
assert(Owlfeather.present_return_followup() == "field_return")
assert(game.state.story_trigger_active_4 == true and
       game.state.story_flag_bit_4 == true)
assert(game.state.field_mode_effect_flag_bit_8 == false and
       game.state.field_mode_effect_flag_bit_9 == false and
       game.state.field_mode_effect_flag_bit_10 == false and
       game.state.field_mode_effect_flag_bit_13 == false and
       game.state.field_mode_effect_flag_bit_14 == true)
assert(services[2][2] == "adjust_source_byte_triplet_2")

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
game.location.id = "salish_red_buffalo_lodge"
game.field.event_code = 1
selected = route(6, "event_1", 1, { 0, 13 })
assert(selected.kind == "field_service" and
       selected.indexed_title_service.module == "salish_red_buffalo")

game.state.story_flag_bit_17 = true
game.state.story_flag_bit_18 = false
game.state.story_trigger_active_4 = nil
game.state.notebook_tip_flag_bit_5 = false
game.state.notebook_tip_flag_bit_6 = true
game.state.field_mode_effect_flag_bit_29 = true
game.state.field_mode_effect_flag_bit_30 = false
choices = { "more", "more" }
assert(RedBuffalo.present_indexed_title_service() == "field_return")
assert(game.state.story_flag_bit_18 == true and
       game.state.story_trigger_active_18 == true)
assert(game.state.notebook_tip_flag_bit_5 == true and
       game.state.notebook_tip_flag_bit_6 == false and
       game.state.field_mode_effect_flag_bit_29 == false and
       game.state.field_mode_effect_flag_bit_30 == true)

-- ROM prompt node 0xF5888 has separate no-business and More outcomes. The
-- former leaves the lodge; the latter advances into the penthouse passage.
game.state.story_flag_bit_17 = false
game.state.story_flag_bit_18 = false
game.state.story_trigger_active_18 = nil
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

print("Salish lodge tests passed: ROM-gated services, dialogue, and same-map return")
