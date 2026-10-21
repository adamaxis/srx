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
        vid_phone_contact_3 = true,
        vid_phone_called_3 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "kipp_david")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_kipp_david")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Kipp menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Kipp title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Kipp label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Kipp menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_kipp_david")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Kipp event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Kipp text " .. tostring(event.title_key))
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
        "Kipp graph did not return to the field terminal")
    assert(#queued_choices == 0, "Kipp graph left unused choices")
end

local function expect_boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Kipp service result must remain a source boundary")
end

local function expect_menu(menu_id)
    for _, menu in ipairs(menus) do
        if menu.id == menu_id then return end
    end
    error("Kipp graph did not present menu " .. menu_id)
end

local function expect_page(event_id)
    for _, page in ipairs(pages) do
        if page == event_id then return end
    end
    error("Kipp graph did not present page " .. event_id)
end

-- The shared caller owns Kipp's first/repeat choices and records his first
-- call before service scope begins.
reset(false, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_kipp_david_first_call")
assert(game.state.vid_phone_called_3 == true and #operations == 0)

reset(true, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_kipp_david_repeat_call")

-- Every information page advances in source order. Services and Other Subject
-- choices reenter the sigh page before the next service selection.
reset(true, {},
      "services", "more", "matrix", "more", "more", "services",
      "more", "cyberdeck", "different_subject",
      "more", "programs", "more", "more", "different_subject",
      "more", "matrix", "more", "more", "goodbye")
expect_finished()
expect_menu("kipp_matrix_intro")
expect_menu("kipp_matrix_corps")
expect_menu("kipp_matrix_result")
expect_menu("kipp_program_intro")
expect_menu("kipp_program_details")
expect_menu("kipp_program_offer")
expect_menu("kipp_deck_offer")

-- Excalibur success shows Kipp's disconnect page and ends the call. A short
-- balance displays the refusal page, then also ends the call.
reset(true, { purchase_excalibur = "deck_installed" },
      "services", "more", "cyberdeck", "pay_for_deck")
expect_finished()
expect_menu("kipp_deck_offer")
expect_page("disconnected")
assert(#operations == 1 and operations[1] == "purchase_excalibur")

reset(true, { purchase_excalibur = "insufficient" },
      "services", "more", "cyberdeck", "pay_for_deck")
expect_finished()
expect_page("insufficient_funds")

-- Rebound and Degrade use separate source actions with shared program-result
-- pages. Insufficient funds ends the call; storage-full and installed results
-- offer Services or Goodbye, with Services returning through the intro.
reset(true, { purchase_rebound = "insufficient" },
      "services", "more", "programs", "more", "more", "buy_rebound")
expect_finished()
expect_page("insufficient_funds")
assert(#operations == 1 and operations[1] == "purchase_rebound")

reset(true, { purchase_degrade = "storage_full" },
      "services", "more", "programs", "more", "more", "buy_degrade",
      "services", "more", "matrix", "more", "more", "goodbye")
expect_finished()
expect_menu("kipp_storage_full")
expect_menu("kipp_matrix_result")
assert(#operations == 1 and operations[1] == "purchase_degrade")

reset(true, { purchase_degrade = "storage_full" },
      "services", "more", "programs", "more", "more", "buy_degrade",
      "goodbye")
expect_finished()
expect_menu("kipp_storage_full")

reset(true, { purchase_rebound = "program_installed" },
      "services", "more", "programs", "more", "more", "buy_rebound",
      "goodbye")
expect_finished()
expect_menu("kipp_program_result")

reset(true, { purchase_degrade = "program_installed" },
      "services", "more", "programs", "more", "more", "buy_degrade",
      "services", "more", "programs", "more", "more",
      "different_subject", "more", "cyberdeck", "different_subject",
      "more", "matrix", "more", "more", "goodbye")
expect_finished()
expect_menu("kipp_program_result")
expect_menu("kipp_deck_offer")
assert(#operations == 1 and operations[1] == "purchase_degrade")

-- Unknown or unavailable host state remains an explicit source boundary.
reset(true, { purchase_excalibur = "unavailable" },
      "services", "more", "cyberdeck", "pay_for_deck")
expect_boundary()

reset(true, { purchase_rebound = "unknown_result" },
      "services", "more", "programs", "more", "more", "buy_rebound")
expect_boundary()

reset(true, { purchase_degrade = "unavailable" },
      "services", "more", "programs", "more", "more", "buy_degrade")
expect_boundary()

print("Kipp David JSON service graph permutations passed")
