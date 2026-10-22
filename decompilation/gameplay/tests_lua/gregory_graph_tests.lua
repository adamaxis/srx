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
    cinematics = load_json("dialogue/cinematics"),
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
        vid_phone_contact_6 = true,
        vid_phone_called_6 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "gregory_wilns")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_gregory_wilns")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Gregory menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Gregory title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Gregory label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Gregory menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_gregory_wilns")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Gregory event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Gregory text " .. tostring(event.title_key))
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
        "Gregory graph did not return to the field terminal")
    assert(#queued_choices == 0, "Gregory graph left unused choices")
end

local function expect_boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Gregory transaction must remain a source boundary")
    assert(#queued_choices == 0, "Gregory boundary left unused choices")
end

local function expect_menu(menu_id)
    for _, menu in ipairs(menus) do
        if menu.id == menu_id then return end
    end
    error("Gregory graph did not present menu " .. menu_id)
end

local function expect_page(event_id)
    for _, page in ipairs(pages) do
        if page == event_id then return end
    end
    error("Gregory graph did not present page " .. event_id)
end

local function expect_transaction(operation)
    assert(#operations == 1 and operations[1] == operation,
        "Gregory graph requested the wrong transaction")
end

-- The common caller still owns first/repeat copy, history, and farewell.
reset(false, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_gregory_wilns_first_call")
assert(game.state.vid_phone_called_6 == true and #operations == 0)
expect_page("goodbye")

reset(true, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_gregory_wilns_repeat_call")

-- The lesson has four ordered pages. Page four's Services route returns to
-- the root; both service offers can then reenter or close the conversation.
reset(true, {},
      "services", "paranormal_creatures", "more", "more", "more", "goodbye")
expect_finished()
for _, menu_id in ipairs({
    "gregory_creature_page_1", "gregory_creature_page_2",
    "gregory_creature_page_3", "gregory_creature_page_4",
}) do
    expect_menu(menu_id)
end
expect_page("goodbye")

reset(true, {},
      "services", "paranormal_creatures", "more", "more", "more",
      "services", "corporate_discount", "decline_discount",
      "magic_enhancement_device", "decline_discount",
      "paranormal_creatures", "more", "more", "more", "goodbye")
expect_finished()
expect_menu("gregory_discount_offer")
expect_menu("gregory_power_focus_offer")
assert(#operations == 0)

-- Talk Later has its own farewell path on each offer.
reset(true, {}, "services", "corporate_discount", "talk_later")
expect_finished()
expect_page("goodbye")

reset(true, {},
      "services", "magic_enhancement_device", "talk_later")
expect_finished()
expect_page("goodbye")

-- Merlin's Lore has insufficient, enabled, and unresolved host results.
reset(true, { purchase_merlins_lore_discount = "insufficient" },
      "services", "corporate_discount", "accept_discount")
expect_finished()
expect_page("insufficient_balance")
expect_transaction("purchase_merlins_lore_discount")

reset(true, { purchase_merlins_lore_discount = "discount_enabled" },
      "services", "corporate_discount", "accept_discount",
      "services", "magic_enhancement_device", "talk_later")
expect_finished()
expect_menu("gregory_discount_result")
expect_page("goodbye")
expect_transaction("purchase_merlins_lore_discount")

reset(true, { purchase_merlins_lore_discount = "discount_enabled" },
      "services", "corporate_discount", "accept_discount", "goodbye")
expect_finished()
expect_menu("gregory_discount_result")
expect_page("goodbye")

reset(true, { purchase_merlins_lore_discount = "unavailable" },
      "services", "corporate_discount", "accept_discount")
expect_boundary()

reset(true, { purchase_merlins_lore_discount = "unknown_result" },
      "services", "corporate_discount", "accept_discount")
expect_boundary()

-- Power Focus distinguishes eligibility, affordability, inventory, purchase,
-- both result-menu continuations, and unresolved native state.
for _, case in ipairs({
    { result = "insufficient", page = "insufficient_balance" },
    { result = "ineligible", page = "power_focus_ineligible" },
    { result = "out_of_stock", page = "power_focus_unavailable" },
}) do
    reset(true, { purchase_power_focus = case.result },
          "services", "magic_enhancement_device", "buy_power_focus")
    expect_finished()
    expect_page(case.page)
    expect_transaction("purchase_power_focus")
end

reset(true, { purchase_power_focus = "purchased" },
      "services", "magic_enhancement_device", "buy_power_focus",
      "services", "paranormal_creatures", "more", "more", "more", "goodbye")
expect_finished()
expect_menu("gregory_power_focus_result")
expect_menu("gregory_creature_page_4")
expect_page("goodbye")
expect_transaction("purchase_power_focus")

reset(true, { purchase_power_focus = "purchased" },
      "services", "magic_enhancement_device", "buy_power_focus", "goodbye")
expect_finished()
expect_menu("gregory_power_focus_result")
expect_page("goodbye")

reset(true, { purchase_power_focus = "unavailable" },
      "services", "magic_enhancement_device", "buy_power_focus")
expect_boundary()

reset(true, { purchase_power_focus = "unknown_result" },
      "services", "magic_enhancement_device", "buy_power_focus")
expect_boundary()

print("Gregory Wilns JSON service graph permutations passed")
