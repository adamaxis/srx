package.path = "decompilation/gameplay/scripts/?.lua;" ..
    "decompilation/gameplay/scripts/?/init.lua;" .. package.path

local Json = require("json")
local function load_json(path)
    local file = assert(io.open("decompilation/gameplay/" .. path .. ".json", "rb"))
    local document = Json.decode(file:read("*a"))
    file:close()
    return document
end

local content = {
    speakers = load_json("dialogue/speakers"),
    menus = load_json("dialogue/menus"),
    contacts = load_json("dialogue/contacts"),
    conditions = load_json("dialogue/conditions"),
    conversations = {
        vid_phone_services = load_json(
            "dialogue/conversations/vid_phone_services"),
    },
}
local text_file = assert(io.open(
    "decompilation/assets/dialogue/script-text.json", "rb"))
local script_text = Json.decode(text_file:read("*a")).texts
text_file:close()

local queued_choices, menus, pages, operations = {}, {}, {}, {}
local service_results = {}

game = {
    content = content,
    state = {},
    location = { type = "field", id = "scene_0_terminal" },
    field = { mode = 0, event_code = 31, alternate_result = 0 },
    ui = {},
    services = {},
}

local function reset(called, results, ...)
    queued_choices = { ... }
    menus, pages, operations = {}, {}, {}
    service_results = results or {}
    game.state = {
        vid_phone_contacts_resolved = true,
        vid_phone_contact_1 = true,
        vid_phone_called_1 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "alesandro_hobbs")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_alesandro_hobbs")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Alesandro menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Alesandro title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Alesandro label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Alesandro menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_alesandro_hobbs")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Alesandro event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Alesandro text " .. tostring(event.title_key))
    pages[#pages + 1] = event_id
    return true
end

game.services.call = function(domain, operation)
    assert(domain == "vid_phone")
    operations[#operations + 1] = operation
    return service_results[operation]
end

local FieldActivities = require("field_activities")

local function run_to_end()
    assert(FieldActivities.vid_phone() == nil,
        "Alesandro graph did not return to the field terminal")
    assert(#queued_choices == 0, "Alesandro graph left unused choices")
end

local function expect_menu(index, menu_id)
    assert(menus[index] ~= nil and menus[index].id == menu_id,
        "expected Alesandro menu " .. menu_id .. " at index " .. index)
end

local function expect_page(index, event_id)
    assert(pages[index] == event_id,
        "expected Alesandro page " .. event_id .. " at index " .. index)
end

-- Common caller owns first/repeat copy and writes the history bit before the
-- first-call Services/Goodbye choices.
reset(false, {}, "goodbye")
run_to_end()
expect_menu(1, "call_alesandro_hobbs_first_call")
assert(game.state.vid_phone_called_1 == true and #operations == 0)

reset(true, {
    purchase_maglock_passkey_level_3 = "purchased",
}, "services", "corporation_information", "patrols",
   "corporation_information", "second_security_type",
   "corporation_information", "avoid_detection", "maglock_passkey",
   "return_to_services", "maglock_passkey", "level_3_passkey", "goodbye")
run_to_end()
expect_menu(1, "call_alesandro_hobbs_repeat_call")
expect_menu(3, "alesandro_security_topics")
expect_menu(9, "alesandro_passkey_offer")
expect_menu(12, "alesandro_purchase_result")
expect_page(1, "security_patrols")
expect_page(2, "security_alerts")
expect_page(3, "security_technological")
expect_page(4, "security_cameras")
assert(#operations == 1 and
       operations[1] == "purchase_maglock_passkey_level_3")

-- Both passkey prices and the official contact use distinct typed host
-- requests; successful results may return to Services or end the call.
reset(true, {
    purchase_maglock_passkey_level_5 = "purchased",
    hire_macklemann = "hired",
}, "services", "maglock_passkey", "level_5_passkey", "services",
   "seattle_official", "pay_for_macklemann", "goodbye")
run_to_end()
expect_menu(4, "alesandro_purchase_result")
expect_menu(6, "alesandro_official_contact_confirmation")
expect_menu(7, "alesandro_macklemann_result")
assert(#operations == 2 and
       operations[1] == "purchase_maglock_passkey_level_5" and
       operations[2] == "hire_macklemann")

-- Insufficient funds end on their page; out-of-stock opens a Services/Goodbye
-- result menu; unavailable and unknown results remain source boundaries.
reset(false, {
    purchase_maglock_passkey_level_3 = "insufficient",
}, "services", "maglock_passkey", "level_3_passkey")
run_to_end()
expect_page(1, "purchase_insufficient")

reset(false, {
    purchase_maglock_passkey_level_5 = "out_of_stock",
}, "services", "maglock_passkey", "level_5_passkey", "goodbye")
run_to_end()
expect_menu(4, "alesandro_out_of_stock")

reset(false, {
    hire_macklemann = "insufficient",
}, "services", "seattle_official", "pay_for_macklemann")
run_to_end()
expect_page(1, "purchase_insufficient")

reset(false, {
    purchase_maglock_passkey_level_3 = "unavailable",
}, "services", "maglock_passkey", "level_3_passkey")
local boundary = FieldActivities.vid_phone()
assert(type(boundary) == "table" and boundary.type == "source_boundary" and
       boundary.reason:find("Alesandro's host transaction", 1, true))

reset(false, {
    hire_macklemann = "unrecognized_result",
}, "services", "seattle_official", "pay_for_macklemann")
boundary = FieldActivities.vid_phone()
assert(type(boundary) == "table" and boundary.type == "source_boundary")

print("Alesandro JSON service graph permutations passed")
