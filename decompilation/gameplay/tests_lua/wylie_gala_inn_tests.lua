package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local Inn = require("wylie_gala_inn")
local location_id = "penumbra_site_09_wylies_gala_inn"
local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = {
        wylie_gala_inn = load_json(
            "dialogue/conversations/wylie_gala_inn"),
    },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local queued_choices = {}
local prompts = {}
local speeches = {}
local service_calls = {}
local payment_result = "paid"

game = {
    content = content,
    state = { runner_feature_flag_bit_2 = false },
    location = { type = "building", id = location_id },
    field = { mode = 3, event_code = 1, alternate_result = 0 },
    ui = {
        prompt_text_key = function(menu_id, text_key, available_choices,
                                   speaker_id, text_profile)
            local profile = content.menus.profiles[text_profile]
            local menu = profile and profile[menu_id]
            assert(menu ~= nil, "unmapped inline prompt: " .. menu_id)
            assert(script_text[text_key] ~= nil,
                "unmapped inline prompt text: " .. text_key)
            assert(speaker_id == "wylie_gala_inn_narrator")
            assert(text_profile == "penumbra.wylies_gala_inn")
            local labels = {}
            for _, choice_id in ipairs(available_choices) do
                local label_key = menu.choice_label_keys[choice_id]
                assert(label_key ~= nil and script_text[label_key] ~= nil,
                    "unmapped Wylie choice label: " .. choice_id)
                labels[#labels + 1] = script_text[label_key]
            end
            prompts[#prompts + 1] = {
                menu_id = menu_id,
                text_key = text_key,
                text = script_text[text_key],
                choice_ids = available_choices,
                labels = labels,
            }
            local selected = table.remove(queued_choices, 1)
            assert(selected ~= nil, "unexpected Wylie prompt: " .. menu_id)
            for _, choice_id in ipairs(available_choices) do
                if choice_id == selected then return selected end
            end
            error("choice " .. selected .. " is not available at " .. menu_id)
        end,
        menu = function()
            error("Wylie inline prompt split its text from the choices")
        end,
        say = function(event_id, speaker_id, text_profile)
            local profile = content.speakers.profiles[speaker_id]
            local event = profile and profile.events[event_id]
            assert(event ~= nil, "unknown Wylie dialogue event: " .. event_id)
            assert(text_profile == "penumbra.wylies_gala_inn")
            local text = script_text[event.title_key]
            assert(text ~= nil, "missing Wylie dialogue text: " .. event_id)
            speeches[#speeches + 1] = { event = event_id, text = text }
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            service_calls[#service_calls + 1] = { domain, operation }
            assert(domain == "building" and operation == "charge_50")
            return payment_result
        end,
    },
}

local function queue(...)
    queued_choices = { ... }
    prompts = {}
    speeches = {}
    service_calls = {}
end

local function expect_outcome(outcome_type, destination_id)
    local outcome = Inn.present_field_exit_prompt()
    assert(outcome.type == outcome_type,
        "expected " .. outcome_type .. ", got " .. tostring(outcome.type))
    if destination_id ~= nil then
        assert(outcome.serviceId == destination_id)
        assert(outcome.returnPolicy == "origin")
    end
    return outcome
end

-- Signed FFF0D5 result selects one of two source page roots before asking
-- whether the party needs a room.
game.field.alternate_result = 1
queue()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #service_calls == 0)
assert(#speeches == 1 and speeches[1].event == "package_after")

game.field.alternate_result = -1
queue()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #service_calls == 0)
assert(#speeches == 1 and speeches[1].event == "package_before")

game.field.alternate_result = nil
queue()
local unknown_run_result = expect_outcome("source_boundary")
assert(unknown_run_result.reason:find("field.alternate_result", 1, true))
assert(#prompts == 0 and #service_calls == 0 and #speeches == 0)

game.field.alternate_result = 0
game.state.runner_feature_flag_bit_2 = false
payment_result = "paid"
queue("crash", "pay")
expect_outcome("open_service", "hotel.options")
assert(#prompts == 2 and prompts[1].text_key == "wylie.gala_inn.entry" and
       prompts[2].text_key == "wylie.gala_inn.room")
assert(prompts[1].labels[1] == "Just looking for a place to crash." and
       prompts[1].labels[2] == "I'll talk to you later, chum.")
assert(prompts[2].labels[1] == "Easy money, chummer.  I'll pay." and
       prompts[2].labels[2] == "I'll talk to you later, chum.")
assert(#service_calls == 1 and service_calls[1][2] == "charge_50")
assert(#speeches == 1 and speeches[1].event == "room_paid")

payment_result = "insufficient"
queue("crash", "pay")
expect_outcome("continue_in_place")
assert(#service_calls == 1 and #speeches == 1 and
       speeches[1].event == "room_insufficient")

-- FF018F bit 2 selects the free-stay result and the same Hotel service.
payment_result = "paid"
game.state.runner_feature_flag_bit_2 = true
queue("crash")
expect_outcome("open_service", "hotel.options")
assert(#prompts == 1 and #service_calls == 0)
assert(#speeches == 1 and speeches[1].event == "free_stay")

-- Missing restored FF018F is unknown, so the graph stops at a boundary.
game.state.runner_feature_flag_bit_2 = nil
queue("crash")
local unknown = expect_outcome("source_boundary")
assert(unknown.reason:find("story.runner_feature_flag_bit_2", 1, true))
assert(#service_calls == 0 and #speeches == 0)

game.state.runner_feature_flag_bit_2 = false
queue("leave")
expect_outcome("continue_in_place")
assert(#service_calls == 0 and #speeches == 0)

queue("crash", "leave")
expect_outcome("continue_in_place")
assert(#service_calls == 0 and #speeches == 0)

payment_result = "service_unavailable"
queue("crash", "pay")
local payment_boundary = expect_outcome("source_boundary")
assert(payment_boundary.reason:find("wallet result", 1, true))

game.field.mode = 2
queue()
local wrong_context = expect_outcome("source_boundary")
assert(wrong_context.reason:find("precondition", 1, true))

print("Wylie's Gala Inn dialogue graph permutations passed")
