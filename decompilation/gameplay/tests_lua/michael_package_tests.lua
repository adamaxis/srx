package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local MichaelPackage = require("michael_package")
local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = {
        stoker_motel = load_json("dialogue/conversations/stoker_motel"),
    },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local queued_choices, choices, pages, services = {}, {}, {}, {}
local service_results = {}
local function reset(...)
    queued_choices = { ... }
    choices, pages, services = {}, {}, {}
end

local function menu(menu_id, available, speaker_id, text_profile)
    assert(speaker_id == "stoker" and text_profile == "stoker.motel")
    local menu_data = content.menus.profiles[text_profile][menu_id]
    assert(menu_data ~= nil, "unmapped Stoker menu " .. menu_id)
    local labels = {}
    for _, id in ipairs(available) do
        local key = menu_data.choice_label_keys[id]
        assert(key ~= nil and script_text[key] ~= nil,
            "unmapped Stoker choice " .. id .. " in " .. menu_id)
        labels[#labels + 1] = script_text[key]
    end
    choices[#choices + 1] = { id = menu_id, options = available, labels = labels }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Stoker menu " .. menu_id)
    for _, id in ipairs(available) do
        if id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game = {
    content = content,
    state = { story_flag_bit_0 = false },
    location = { type = "motel", id = "stoker_motel" },
    field = { mode = 2, event_code = 2, alternate_result = 0 },
    ui = {
        menu = menu,
        prompt_text_key = function()
            error("Stoker graph unexpectedly split inline prompt text")
        end,
        say = function(event_id, speaker_id, text_profile)
            local speaker = content.speakers.profiles[speaker_id]
            local event = speaker and speaker.events[event_id]
            assert(event ~= nil, "unknown Stoker event " .. event_id)
            assert(text_profile == "stoker.motel")
            local key = event.title_key
            assert(script_text[key] ~= nil, "missing Stoker text " .. key)
            pages[#pages + 1] = { id = event_id, key = key }
            return true
        end,
    },
    services = {
        call = function(domain, operation)
            services[#services + 1] = { domain = domain, operation = operation }
            local key = domain .. "." .. operation
            return service_results[key]
        end,
    },
}

local function outcome(result)
    assert(result.type == "continue_in_place" or
           result.type == "open_service" or
           result.type == "source_boundary",
        "unexpected Stoker outcome " .. tostring(result.type))
    return result
end

local function expect_page(index, event_id)
    assert(pages[index] ~= nil and pages[index].id == event_id,
        "expected Stoker page " .. event_id)
end

-- Source gate and ordered A/B/C menu: the package choice is present while
-- FFFC1E trigger bit 0 is clear. There is no invented opening_dialogue_seen
-- save fact controlling the page sequence.
reset("crash", "pay")
service_results["building.charge_20"] = "paid"
local result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "open_service" and result.serviceId == "hotel.options" and
       result.returnPolicy == "origin")
assert(game.state.story_trigger_active_0 == true)
expect_page(1, "arrival")
expect_page(2, "room.paid")
assert(choices[1].id == "manager" and #choices[1].options == 3 and
       choices[1].options[1] == "crash" and
       choices[1].options[2] == "leave" and
       choices[1].options[3] == "package")
assert(choices[1].labels[3] == "I'm here to pick up Michael's stuff.")
assert(#services == 1 and services[1].domain == "building" and
       services[1].operation == "charge_20")

-- Room insufficient funds and both Leave edges stay on the same field map.
service_results["building.charge_20"] = "insufficient"
reset("crash", "pay")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "room.insufficient")

reset("leave")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "leave")

reset("crash", "leave")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "leave")

-- Michael branch: command 0x30 precedes the debt question, and command 0x05
-- branches into either the package handoff or Jump House low-funds page.
service_results["michael_package.prepare_debt_offer"] = "ready"
service_results["michael_package.pay_debt"] = "paid"
reset("package", "debt-payment")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place" and
       game.state.story_flag_bit_0 == true)
expect_page(2, "package-handoff")
assert(services[1].operation == "prepare_debt_offer" and
       services[2].operation == "pay_debt")

game.state.story_flag_bit_0 = false
service_results["michael_package.pay_debt"] = "insufficient"
reset("package", "debt-payment")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place" and
       game.state.story_flag_bit_0 == false)
expect_page(2, "jump-house")

service_results["michael_package.jump_house_referral"] = "ready"
reset("package", "jump-house")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "continue_in_place")
expect_page(2, "jump-house")
assert(services[2].operation == "jump_house_referral")

-- Unknown source facts and unavailable host transactions stop at explicit
-- boundaries rather than choosing a branch or claiming a successful return.
game.state.story_flag_bit_0 = nil
reset()
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "source_boundary" and
       result.reason:find("story.story_flag_bit_0", 1, true))
assert(#choices == 0 and #pages == 0 and #services == 0)

game.state.story_flag_bit_0 = false
service_results["building.charge_20"] = nil
reset("crash", "pay")
result = outcome(MichaelPackage.stoker_interaction())
assert(result.type == "source_boundary" and
       result.reason:find("wallet result", 1, true))

-- The scene-2/submode-2 selector keeps signed alternate results distinct.
game.field.alternate_result = 1
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place" and #choices == 0)
expect_page(1, "package-question")

game.field.alternate_result = -1
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place" and #choices == 0)
expect_page(1, "package-handoff")

game.field.alternate_result = 0
service_results["stoker_motel.resolve_arrival"] = "manager"
reset("leave")
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "continue_in_place")
expect_page(1, "arrival")
assert(choices[1].id == "arrival_manager" and
       #choices[1].options == 2)
expect_page(2, "leave")

service_results["stoker_motel.resolve_arrival"] = "street_ambush"
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("actor/effect handoff", 1, true))
expect_page(2, "street-ambush")

service_results["stoker_motel.resolve_arrival"] = nil
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("arrival command", 1, true))
expect_page(1, "arrival")

game.field.alternate_result = nil
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("field.alternate_result", 1, true))
assert(#pages == 0 and #choices == 0)

game.field.alternate_result = 0
game.field.mode = 1
reset()
result = outcome(MichaelPackage.present_field_exit_prompt())
assert(result.type == "source_boundary" and
       result.reason:find("precondition", 1, true))

print("Stoker dialogue graph permutations passed")
