package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local source = file:read("*a")
    file:close()
    return Json.decode(source)
end

local Inn = require("tarislar_city_inn")

local location_id = "puyallup_site_03_tarislar_city_inn"
local choices = {}
local prompts = {}
local speeches = {}
local service_calls = {}
local payment_result = "paid"
local triplet_result = "applied"
local script_text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text_catalog = Json.decode(script_text_file:read("*a"))
script_text_file:close()
local script_text = script_text_catalog.texts

local function take_choice(id, available_choices)
    local selected = table.remove(choices, 1)
    assert(selected ~= nil, "unexpected prompt: " .. id)
    local allowed = false
    for _, option in ipairs(available_choices) do
        if option == selected then allowed = true end
    end
    assert(allowed, "choice " .. selected .. " is not available at " .. id)
    return selected
end

game = {
    state = {
        story_flag_bit_0 = true,
        story_flag_bit_1 = false,
        story_flag_bit_13 = false,
        story_flag_bit_30 = false,
    },
    location = { type = "building", id = location_id },
    field = { mode = 1, event_code = 1, alternate_result = 0 },
    content = {
        speakers = load_json("dialogue/speakers"),
        menus = load_json("dialogue/menus"),
        conditions = load_json("dialogue/conditions"),
        cinematics = load_json("dialogue/cinematics"),
        conversations = {
            tarislar_city_inn = load_json(
                "dialogue/conversations/tarislar_city_inn"),
        },
    },
    ui = {
        prompt_text = function(id, text, available_choices)
            prompts[#prompts + 1] = {
                id = id,
                text = text,
                choices = available_choices,
            }
            return take_choice(id, available_choices)
        end,
        say = function(event_id, speaker_id, text_profile, presentation,
                       input_delay_frames)
            local profile = game.content.speakers.profiles[speaker_id]
            local event = profile and profile.events[event_id]
            assert(event ~= nil, "unknown dialogue event: " .. event_id)
            speeches[#speeches + 1] = {
                event = event_id,
                text = script_text[event.title_key],
                continuation = event.continuation_key == "shared.more",
                speaker_id = speaker_id,
                text_profile = text_profile,
                presentation = presentation,
                input_delay_frames = input_delay_frames,
            }
            assert(speeches[#speeches].text ~= nil,
                "dialogue event has no generated text: " .. event.title_key)
            if event.continuation_key == "shared.more" then
                assert(table.remove(choices, 1) == "more",
                    "expected MORE after " .. event_id)
            end
            return true
        end,
        menu = function(id, available_choices, speaker_id, text_profile)
            local profile = game.content.menus.profiles[text_profile]
            local definition = profile and profile[id]
            assert(definition ~= nil, "unknown dialogue menu: " .. id)
            local labels = {}
            for _, choice_id in ipairs(available_choices) do
                local label_key = definition.choice_label_keys[choice_id]
                labels[#labels + 1] = script_text[label_key]
                assert(labels[#labels] ~= nil,
                    "dialogue menu has no generated choice text: " ..
                    tostring(label_key))
            end
            prompts[#prompts + 1] = {
                id = id,
                choices = available_choices,
                labels = labels,
                text = script_text[definition.title_key],
            }
            return take_choice(id, available_choices)
        end,
        say_text = function(text, continuation)
            speeches[#speeches + 1] = {
                text = text,
                continuation = continuation == true,
            }
        end,
    },
    services = {
        call = function(domain, operation)
            service_calls[#service_calls + 1] = { domain, operation }
            if domain == "building" and operation == "charge_30" then
                return payment_result
            end
            if domain == "runner" and
               operation == "adjust_source_byte_triplet_1" then
                return triplet_result
            end
            return nil
        end,
    },
}

local function queue(...)
    choices = { ... }
    prompts = {}
    speeches = {}
    service_calls = {}
end

local function location_task_result(expected, ...)
    queue(...)
    local outcome = Inn.present_indexed_title_service()
    assert(type(outcome) == "table" and outcome.type == expected,
        "unexpected Tarislar graph outcome: " .. tostring(outcome))
    assert(#choices == 0, "unused menu choice")
    return outcome
end

-- The indexed title-service prerequisites come from scene/event and FFFC1E.
game.location.id = "another_building"
queue("more")
assert(Inn.present_indexed_title_service() == "denied")
assert(#prompts == 0)
game.location.id = location_id
game.field.mode = 2
assert(Inn.present_indexed_title_service() == "denied")
game.field.mode = 1

-- The ordinary selector exposes Frosty only for FFFC1E bit 13 and a clear
-- FFFC1E bit 30. It keeps the ROM's stay-here result.
queue("more", "leave")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#prompts[2].choices == 2 and prompts[2].choices[2] == "leave")
game.state.story_flag_bit_13 = true
queue("more", "frosty")
assert(Inn.present_field_exit_prompt() == "field_return")
assert(prompts[2].choices[2] == "frosty")
assert(game.state.notebook_tip_flag_bit_23 == true)
assert(game.state.field_mode_effect_flag_bit_4 == true)
assert(game.state.field_mode_effect_flag_bit_13 == true)
assert(game.state.field_mode_effect_flag_bit_21 == true)
game.state.story_flag_bit_13 = false

-- Scene 1's signed alternate result selects the two package text nodes.
game.field.alternate_result = 1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1].text:find("package", 1, true))
game.field.alternate_result = -1
queue()
assert(Inn.present_field_exit_prompt() == "field_return")
assert(#speeches == 1 and speeches[1].text:find("roots around", 1, true))
game.field.alternate_result = 0

-- Room payment uses the generic building service and returns the hotel menu
-- only after a successful 30-nuyen transaction.
payment_result = "paid"
local paid_stay = location_task_result(
    "open_service", "more", "crash", "pay")
assert(paid_stay.serviceId == "hotel.options" and
       paid_stay.returnPolicy == "origin")
assert(#service_calls == 1 and service_calls[1][1] == "building" and
       service_calls[1][2] == "charge_30")
assert(speeches[#speeches].event == "room_paid" and
       speeches[#speeches].text:find("towels", 1, true))
payment_result = "insufficient"
location_task_result("continue_in_place", "more", "crash", "pay")
assert(#service_calls == 1 and service_calls[1][2] == "charge_30")
assert(speeches[#speeches].event == "room_insufficient" and
       speeches[#speeches].text:find("low on creds", 1, true))

-- Leave and decline both close on the exterior map without following scene 53.
location_task_result("continue_in_place", "more", "leave")
assert(speeches[#speeches].event == "leave")
location_task_result("continue_in_place", "more", "crash", "decline")
assert(speeches[#speeches].event == "leave")

-- The ROM prompt chain is represented as JSON pages and menus. Command 0x16
-- marks the active trigger before the first page; command 0x14's typed source
-- triplet call and story masks happen only after Tabatha agrees to receive
-- the runner.
game.state.story_flag_bit_1 = false
game.state.story_trigger_active_1 = nil
triplet_result = "applied"
local refusal_route = location_task_result("continue_in_place",
    "more", "shale", "more", "more", "more", "about_michael", "more",
    "caleb", "errascoe", "recruit", "more", "more", "more", "more")
assert(game.state.story_trigger_active_1 == true)
assert(game.state.story_flag_bit_1 == true)
assert(game.state.notebook_tip_flag_bit_2 == true)
assert(game.state.notebook_tip_flag_bit_4 == true)
assert(game.state.notebook_tip_flag_bit_5 == true)
assert(game.state.notebook_tip_flag_bit_6 == true)
assert(game.state.notebook_tip_flag_bit_8 == true)
assert(game.state.field_mode_effect_flag_bit_2 == true)
assert(game.state.field_mode_effect_flag_bit_4 == true)
assert(game.state.mr_johnsons_flag_bit_2 == true)
assert(game.state.mr_johnsons_flag_bit_4 == true)
assert(service_calls[1][1] == "runner" and
       service_calls[1][2] == "adjust_source_byte_triplet_1")
assert(#speeches == 10 and speeches[1].event == "entry" and
       speeches[2].event == "shale_intro_0" and
       speeches[3].event == "shale_intro_1" and
       speeches[4].event == "shale_intro_2" and
       speeches[5].event == "shale_last_run" and
       speeches[6].event == "shale_refuses_recruitment" and
       speeches[7].event == "shale_attack" and
       speeches[8].event == "attack_aftermath" and
       speeches[9].event == "room_reentry" and
       speeches[10].event == "holopix_revealed")
assert(speeches[1].speaker_id == "tarislar_city_inn_manager" and
       speeches[5].speaker_id == "tarislar_city_inn_shale" and
       speeches[8].speaker_id == "tarislar_city_inn_renraku" and
       speeches[9].speaker_id == "tarislar_city_inn_shale" and
       speeches[10].speaker_id == "tarislar_city_inn_david_owlfeather")
assert(speeches[2].input_delay_frames == 12 and
       speeches[7].input_delay_frames == 12 and
       speeches[10].presentation == "field_mode_text" and
       speeches[10].input_delay_frames == 30)
assert(speeches[6].continuation == true and
       speeches[7].continuation == true and
       speeches[8].continuation == true and
       speeches[9].continuation == true and
       speeches[10].continuation == false)
assert(prompts[2].id == "shale_arrival" and
       prompts[2].text == script_text["tarislar.city_inn.shale.apartment"])
assert(prompts[3].id == "shale_topics" and
       prompts[3].text == script_text["tarislar.city_inn.shale.caleb"])
assert(prompts[4].id == "shale_after_caleb" and
       prompts[4].text ==
           script_text["tarislar.city_inn.shale.caleb_followup"] and
       #prompts[4].choices == 1 and prompts[4].choices[1] == "errascoe")
assert(prompts[5].id == "shale_followup_after_caleb" and
       prompts[5].text == script_text["tarislar.city_inn.shale.errascoe"] and
       #prompts[5].choices == 2 and prompts[5].choices[1] == "recruit" and
       prompts[5].choices[2] == "thanks")
assert(Inn.present_indexed_title_service() == "denied",
       "the one-shot Tabatha service ran after FFFC1E bit 1 was set")

-- The reverse topic permutation converges on Caleb's account and the same
-- attack page when the runner thanks Tabatha.
game.state.story_flag_bit_1 = false
game.state.story_flag_bit_30 = false
game.state.story_flag_bit_13 = false
triplet_result = "applied"
location_task_result("continue_in_place",
    "more", "shale", "more", "more", "more", "about_michael", "more",
    "errascoe", "caleb", "thanks", "more", "more", "more")
assert(speeches[7].event == "shale_attack" and
       speeches[8].event == "attack_aftermath" and
       speeches[9].event == "room_reentry" and
       speeches[10].event == "holopix_revealed" and
       speeches[10].speaker_id == "tarislar_city_inn_david_owlfeather")
assert(prompts[4].id == "shale_after_errascoe" and
       prompts[4].text == script_text["tarislar.city_inn.shale.errascoe"] and
       #prompts[4].choices == 1 and prompts[4].choices[1] == "caleb")
assert(prompts[5].id == "shale_followup_after_errascoe" and
       prompts[5].text ==
           script_text["tarislar.city_inn.shale.caleb_followup"] and
       #prompts[5].choices == 2 and prompts[5].choices[1] == "recruit" and
       prompts[5].choices[2] == "thanks")

-- If the session cannot apply command 0x14's source byte update, the script
-- reports that failure instead of claiming acceptance completed.
game.state.story_flag_bit_1 = false
triplet_result = "unavailable"
local state_boundary = location_task_result("source_boundary",
    "more", "shale", "more", "more", "more")
assert(state_boundary.reason:find("0x21862", 1, true))

print("Tarislar City Inn tests passed: gates, payment, dialogue profiles, and Tabatha aftermath")
