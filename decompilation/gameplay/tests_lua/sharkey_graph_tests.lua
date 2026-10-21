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
        vid_phone_contact_2 = true,
        vid_phone_called_2 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "sharkey")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_sharkey")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Sharkey menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Sharkey title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Sharkey label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Sharkey menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_sharkey")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Sharkey event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Sharkey text " .. tostring(event.title_key))
    pages[#pages + 1] = event_id
    return true
end

game.services.call = function(domain, operation)
    assert(domain == "vid_phone")
    operations[#operations + 1] = operation
    return service_results[operation]
end

local FieldActivities = require("field_activities")

local function expect_finished()
    assert(FieldActivities.vid_phone() == nil,
        "Sharkey graph did not return to the field terminal")
    assert(#queued_choices == 0, "Sharkey graph left unused choices")
end

local function expect_scene(scene_id)
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and
           outcome.type == "enter_field_scene" and
           outcome.sceneId == scene_id and
           outcome.arrivalId == "rom_default",
        "Sharkey passage selected the wrong typed scene outcome")
    assert(#queued_choices == 0, "Sharkey graph left unused choices")
end

local function expect_boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Sharkey service result must remain a source boundary")
end

-- The shared caller owns first/repeat handling and records the first call
-- before offering Services/Goodbye.
reset(false, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_sharkey_first_call")
assert(game.state.vid_phone_called_2 == true and #operations == 0)

reset(true, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_sharkey_repeat_call")

-- Both fares use the route table's encoded operation, show the shared result
-- page, then end in their own ROM-default arrival scene.
reset(true, {
    ["passage_to_scene_6.fare_3000"] = "passage_requested",
}, "services", "pass_salish", "pay_for_passage")
expect_scene(6)
assert(pages[1] == "passage_result" and #operations == 1 and
       operations[1] == "passage_to_scene_6.fare_3000")

reset(true, {
    ["passage_to_scene_9.fare_5000"] = "passage_requested",
}, "services", "pass_sinsearach", "pay_for_passage")
expect_scene(9)
assert(pages[1] == "passage_result" and #operations == 1 and
       operations[1] == "passage_to_scene_9.fare_5000")

-- Every non-purchase offer edge returns to Services or ends the call. A
-- failed fare displays its result and the graph resumes the Services root.
reset(true, {
    ["passage_to_scene_6.fare_3000"] = "insufficient",
}, "services", "pass_salish", "pay_for_passage", "smart_goggles",
   "return_to_services", "pass_sinsearach", "other_subject")
expect_finished()
assert(pages[1] == "insufficient_funds" and
       operations[1] == "passage_to_scene_6.fare_3000" and
       operations[2] == nil)

reset(true, {
    ["passage_to_scene_9.fare_5000"] = "insufficient",
}, "services", "pass_sinsearach", "pay_for_passage", "smart_goggles",
   "other_subject")
expect_finished()
assert(pages[1] == "insufficient_funds" and
       operations[1] == "passage_to_scene_9.fare_5000")

reset(true, {}, "services", "pass_salish", "return_to_services",
   "pass_sinsearach", "return_to_services", "smart_goggles",
   "other_subject")
expect_finished()
assert(#operations == 0)

reset(true, {}, "services", "smart_goggles", "return_to_services",
   "pass_salish", "other_subject")
expect_finished()
assert(#operations == 0)

-- Smart Goggles success, insufficient funds, and full inventory each show
-- their source page and return to the shared service list.
reset(true, { purchase_smart_goggles = "purchased" },
      "services", "smart_goggles", "confirm_purchase", "pass_salish",
      "other_subject")
expect_finished()
assert(pages[1] == "smart_goggles_result" and
       operations[1] == "purchase_smart_goggles")

reset(true, { purchase_smart_goggles = "insufficient" },
      "services", "smart_goggles", "confirm_purchase", "pass_salish",
      "other_subject")
expect_finished()
assert(pages[1] == "insufficient_funds")

reset(true, { purchase_smart_goggles = "out_of_stock" },
      "services", "smart_goggles", "confirm_purchase", "pass_salish",
      "other_subject")
expect_finished()
assert(pages[1] == "smart_goggles_inventory_full")

-- Unavailable and malformed host results stop at explicit source boundaries.
reset(true, {
    ["passage_to_scene_6.fare_3000"] = "unavailable",
}, "services", "pass_salish", "pay_for_passage")
expect_boundary()

reset(true, {
    ["passage_to_scene_9.fare_5000"] = "unexpected_result",
}, "services", "pass_sinsearach", "pay_for_passage")
expect_boundary()

reset(true, { purchase_smart_goggles = "unexpected_result" },
      "services", "smart_goggles", "confirm_purchase")
expect_boundary()

reset(true, { purchase_smart_goggles = "unavailable" },
      "services", "smart_goggles", "confirm_purchase")
expect_boundary()

print("Sharkey JSON service graph permutations passed")
