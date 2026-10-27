package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local Inn = require("council_island_inn")
local speakers = load_json("dialogue/speakers").profiles
local menus = load_json("dialogue/menus").profiles
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local queued_choices, prompts, speeches, calls = {}, {}, {}, {}
local payment_result = "paid"
game = {
    location = { type = "building", id = "council_island_inn" },
    field = { mode = 4, event_code = 0, alternate_result = 0 },
    ui = {
        prompt_text = function(menu_id, text, available_choices)
            local menu = assert(menus.council_island_inn[menu_id],
                "unknown Council Island Inn prompt: " .. menu_id)
            local labels = {}
            for _, id in ipairs(available_choices) do
                local key = assert(menu.choice_label_keys[id],
                    "unmapped Council Island Inn choice: " .. id)
                labels[#labels + 1] = assert(script_text[key],
                    "missing Council Island Inn choice text: " .. key)
            end
            prompts[#prompts + 1] = {
                id = menu_id, text = text, choice_ids = available_choices,
                labels = labels,
            }
            local selected = table.remove(queued_choices, 1)
            assert(selected ~= nil, "unexpected Council Island Inn prompt")
            for _, id in ipairs(available_choices) do
                if id == selected then return selected end
            end
            error("choice " .. selected .. " is unavailable in " .. menu_id)
        end,
        say_text = function(text, continuation)
            speeches[#speeches + 1] = {
                text = text, continuation = continuation,
            }
            return true
        end,
        say = function(event_id, speaker_id, text_profile)
            local speaker = assert(speakers[speaker_id],
                "unknown Council Island Inn speaker: " .. speaker_id)
            assert(text_profile == "council_island_inn" and
                   speaker.text_profile == text_profile)
            local event = assert(speaker.events[event_id],
                "unknown Council Island Inn event: " .. event_id)
            speeches[#speeches + 1] = {
                event = event_id, text = assert(script_text[event.title_key]),
            }
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            calls[#calls + 1] = { domain = domain, operation = operation }
            assert(domain == "building" and operation == "charge_40")
            return payment_result
        end,
    },
}

local function reset(...)
    queued_choices = { ... }
    prompts, speeches, calls = {}, {}, {}
end

local function expect_outcome(kind, service_id)
    local result = Inn.present_field_exit_prompt()
    assert(type(result) == "table" and result.type == kind,
        "expected " .. kind .. ", got " .. tostring(result and result.type))
    if service_id ~= nil then
        assert(result.serviceId == service_id and
               result.returnPolicy == "origin")
    end
    return result
end

-- ROM 0x1F7F6/0x1F800 select the signed package roots before the default
-- room offer at 0xF4A60. Neither signed branch charges for a room.
game.field.alternate_result = 1
reset()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #calls == 0 and #speeches == 1)
assert(speeches[1].text:find("well-dressed elf", 1, true) and
       speeches[1].text:find("gives you the payment", 1, true))

game.field.alternate_result = -1
reset()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #calls == 0 and #speeches == 1)
assert(speeches[1].text:find("tell the proprietor", 1, true) and
       speeches[1].text:find("finds it", 1, true))

-- The two byte endpoints must take the same ROM sign branches; malformed
-- values outside the signed byte domain stop at an explicit boundary.
game.field.alternate_result = 127
reset()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #calls == 0 and #speeches == 1 and
       speeches[1].text:find("well-dressed elf", 1, true))
game.field.alternate_result = -128
reset()
expect_outcome("continue_in_place")
assert(#prompts == 0 and #calls == 0 and #speeches == 1 and
       speeches[1].text:find("tell the proprietor", 1, true))

local function expect_invalid_alternate_result(value)
    game.field.alternate_result = value
    reset()
    local result = expect_outcome("source_boundary")
    assert(result.reason:find("signed alternate-result byte", 1, true))
    assert(#prompts == 0 and #calls == 0 and #speeches == 0)
end
expect_invalid_alternate_result(nil)
expect_invalid_alternate_result(1.5)
expect_invalid_alternate_result(128)
expect_invalid_alternate_result(-129)
expect_invalid_alternate_result("1")

game.field.alternate_result = 0
game.field.mode = 3
reset()
local wrong_mode = expect_outcome("source_boundary")
assert(wrong_mode.reason:find("mode 4/event 0", 1, true))
assert(#prompts == 0 and #calls == 0 and #speeches == 0)
game.field.mode = 4
game.location.id = "other_building"
reset()
local wrong_location = expect_outcome("source_boundary")
assert(wrong_location.reason:find("mode 4/event 0", 1, true))
assert(#prompts == 0 and #calls == 0 and #speeches == 0)
game.location.id = "council_island_inn"
game.field.event_code = 1
reset()
local wrong_event = expect_outcome("source_boundary")
assert(wrong_event.reason:find("mode 4/event 0", 1, true))
assert(#prompts == 0 and #calls == 0 and #speeches == 0)
game.field.event_code = 0

-- Leave at either prompt keeps the party on Scene 4 and does not touch funds.
reset("leave")
expect_outcome("continue_in_place")
assert(prompts[1].text:find("Salutations, weary traveler!", 1, true))
assert(prompts[1].labels[1] == "Just looking for a place to crash." and
       prompts[1].labels[2] == "I'll talk to you later, chum.")
assert(#calls == 0 and #speeches == 0)

reset("crash", "leave")
expect_outcome("continue_in_place")
assert(prompts[2].text:find("40 nuyen", 1, true))
assert(prompts[2].labels[1] == "Easy money, chummer.  I'll pay." and
       prompts[2].labels[2] == "I'll talk to you later, chum.")
assert(#calls == 0 and #speeches == 0)

-- ROM 0x21458 charges exactly 40, then 0x1F81A enters Hotel Options.
payment_result = "paid"
reset("crash", "pay")
expect_outcome("open_service", "hotel.options")
assert(#calls == 1 and calls[1].domain == "building" and
       calls[1].operation == "charge_40")
assert(#speeches == 1 and speeches[1].event == "room_paid" and
       speeches[1].text:find("Hope you brought", 1, true))

-- A wallet below 40 remains unchanged in the native charge service and the
-- source returns the low-creds page without entering Hotel Options.
payment_result = "insufficient"
reset("crash", "pay")
expect_outcome("continue_in_place")
assert(#calls == 1 and #speeches == 1 and
       speeches[1].event == "room_insufficient" and
       speeches[1].text:find("stick seems to be low", 1, true))

payment_result = "service_unavailable"
reset("crash", "pay")
local unresolved = expect_outcome("source_boundary")
assert(unresolved.reason:find("wallet result", 1, true))
assert(#calls == 1 and #speeches == 0)

print("council_island_inn_tests=passed payment=40 signed_results=both")
