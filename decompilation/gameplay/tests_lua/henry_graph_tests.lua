package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local value = Json.decode(file:read("*a"))
    file:close()
    return value
end

local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    contacts = load_json("dialogue/contacts"),
    conditions = load_json("dialogue/conditions"),
    cinematics = load_json("dialogue/cinematics"),
    conversations = { vid_phone_services = load_json(
        "dialogue/conversations/vid_phone_services") },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local texts = Json.decode(text_file:read("*a")).texts
text_file:close()

local queued, shown_menus, shown_pages, calls, results = {}, {}, {}, {}, {}
game = { content = content, state = {}, location = { type = "field" },
         field = {}, ui = {}, services = {} }

local function reset(called, service_results, ...)
    queued, shown_menus, shown_pages, calls = { ... }, {}, {}, {}
    results = service_results or {}
    game.state = {
        vid_phone_contacts_resolved = true,
        vid_phone_contact_9 = true,
        vid_phone_called_9 = called,
        lone_star_criminal_record_active = true,
        vid_phone_story_flag_bit_27 = false,
        group_item_flag_bit_11 = false,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected, speaker, profile)
    assert(menu_id == "contacts" and selected == 1)
    assert(speaker == "" and profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "henry_j_culver")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker, profile)
    assert(speaker == "vid_phone_henry_j_culver")
    assert(profile == "field.vid_phone")
    local definition = content.menus.profiles[profile][menu_id]
    assert(definition, "missing Henry menu " .. menu_id)
    assert(definition.title_key == "" or texts[definition.title_key],
        "missing Henry title " .. tostring(definition.title_key))
    for _, id in ipairs(choices) do
        local key = definition.choice_label_keys[id]
        assert(key and texts[key], "missing Henry label " .. menu_id .. "." .. id)
    end
    shown_menus[#shown_menus + 1] = menu_id
    local selected = table.remove(queued, 1)
    assert(selected, "unexpected Henry menu " .. menu_id)
    for _, id in ipairs(choices) do if id == selected then return selected end end
    error("unexpected choice " .. selected .. " at " .. menu_id)
end

game.ui.say = function(event_id, speaker, profile)
    assert(speaker == "vid_phone_henry_j_culver")
    assert(profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker].events[event_id]
    assert(event and texts[event.title_key], "missing Henry event " .. event_id)
    shown_pages[#shown_pages + 1] = event_id
    return true
end

game.services.call = function(domain, operation)
    assert(domain == "building")
    calls[#calls + 1] = operation
    return results[operation]
end

local FieldActivities = require("field_activities")
local function finish()
    assert(FieldActivities.vid_phone() == nil,
        "Henry graph did not return from the Vid-Phone")
    assert(#queued == 0, "Henry graph left unused choices")
end
local function boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unknown Henry source result must remain a boundary")
end
local function has(list, expected)
    for _, value in ipairs(list) do if value == expected then return true end end
    return false, "missing " .. expected
end

reset(false, {}, "goodbye")
finish()
assert(game.state.vid_phone_called_9 == true and has(shown_menus,
    "call_henry_j_culver_first_call"))
assert(has(shown_pages, "goodbye"))

reset(true, {}, "goodbye")
finish()
assert(has(shown_menus, "call_henry_j_culver_repeat_call"))

local offers = {
    { id = "delete_record", menu = "henry_offer_delete_record",
      operation = "charge_2500", result = "record_deleted",
      fact = "lone_star_criminal_record_active", value = false },
    { id = "hollywood_access", menu = "henry_offer_hollywood_access",
      operation = "charge_1500", result = "hollywood_access_granted",
      fact = "vid_phone_story_flag_bit_27", value = true },
    { id = "weapon_permit", menu = "henry_offer_weapon_permit",
      operation = "charge_8000", result = "weapon_permit_granted",
      fact = "group_item_flag_bit_11", value = true },
}

for _, offer in ipairs(offers) do
    reset(true, {}, "services", offer.id, "decline", "goodbye")
    finish()
    assert(#calls == 0, "declining " .. offer.id .. " charged a fee")
    assert(has(shown_menus, "henry_service_return"))

    reset(true, { [offer.operation] = "paid" },
          "services", offer.id, "accept")
    finish()
    assert(#calls == 1 and calls[1] == offer.operation)
    assert(game.state[offer.fact] == offer.value,
        "paid " .. offer.id .. " did not apply its ROM fact")
    assert(has(shown_pages, offer.result) and has(shown_pages, "goodbye"))

    reset(true, { [offer.operation] = "insufficient" },
          "services", offer.id, "accept")
    finish()
    assert(#calls == 1 and calls[1] == offer.operation)
    assert(game.state[offer.fact] ~= offer.value,
        "failed " .. offer.id .. " changed its ROM fact")
    assert(has(shown_pages, "payment_insufficient"))

    reset(true, { [offer.operation] = "unknown" },
          "services", offer.id, "accept")
    boundary()
end

print("Henry J. Culver call and service graph permutations passed")
