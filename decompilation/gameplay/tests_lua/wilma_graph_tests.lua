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
        vid_phone_contact_4 = true,
        vid_phone_called_4 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "wilma_temmenhoff")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_wilma_temmenhoff")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Wilma menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Wilma title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Wilma label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Wilma menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_wilma_temmenhoff")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Wilma event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Wilma text " .. tostring(event.title_key))
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
        "Wilma graph did not return to the field terminal")
    assert(#queued_choices == 0, "Wilma graph left unused choices")
end

local function expect_boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Wilma service result must remain a source boundary")
    assert(#queued_choices == 0, "Wilma boundary left unused choices")
end

local function expect_menu(menu_id)
    for _, menu in ipairs(menus) do
        if menu.id == menu_id then return end
    end
    error("Wilma graph did not present menu " .. menu_id)
end

local function count_menus(menu_id)
    local count = 0
    for _, menu in ipairs(menus) do
        if menu.id == menu_id then count = count + 1 end
    end
    return count
end

local function expect_page(event_id)
    for _, page in ipairs(pages) do
        if page == event_id then return end
    end
    error("Wilma graph did not present page " .. event_id)
end

local function service_choices(action_choice)
    return {
        "services", "more", "programs", "more", "more", action_choice,
    }
end

local function append_choices(base, ...)
    local result = {}
    for _, choice in ipairs(base) do result[#result + 1] = choice end
    for _, choice in ipairs({ ... }) do result[#result + 1] = choice end
    return result
end

-- The shared contact menu still owns first/repeat call history.
reset(false, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_wilma_temmenhoff_first_call")
assert(game.state.vid_phone_called_4 == true and #operations == 0)

reset(true, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_wilma_temmenhoff_repeat_call")

-- Every Matrix information page advances in source order. Its Services edge
-- repeats the credstick page; Goodbye returns directly to the service menu.
reset(true, {},
      "services", "more", "matrix", "more", "more", "more",
      "services", "more", "goodbye")
expect_finished()
expect_menu("wilma_matrix_intro")
expect_menu("wilma_matrix_companies")
expect_menu("wilma_matrix_sleaze")
expect_menu("wilma_matrix_result")
assert(count_menus("wilma_credstick") == 2)
assert(count_menus("wilma_services") == 2)
expect_page("later")

reset(true, {},
      "services", "more", "matrix", "more", "more", "more",
      "goodbye", "goodbye")
expect_finished()
assert(count_menus("wilma_credstick") == 1)
assert(count_menus("wilma_services") == 2)
expect_page("later")

-- The different-subject branch returns directly to Services.
reset(true, {},
      "services", "more", "programs", "more", "more",
      "different_subject", "goodbye")
expect_finished()
expect_menu("wilma_program_offer")
expect_page("later")

-- Both ROM commands use the same result dialogue but retain separate action
-- identities. Cover each source result and both result-menu continuations.
for _, case in ipairs({
    { choice = "buy_rebound", operation = "purchase_wilma_rebound" },
    { choice = "buy_degrade", operation = "purchase_wilma_degrade" },
}) do
    local choices = service_choices(case.choice)
    reset(true, { [case.operation] = "insufficient" }, table.unpack(choices))
    expect_finished()
    expect_page("insufficient_funds")
    assert(#operations == 1 and operations[1] == case.operation)

    choices = service_choices(case.choice)
    for _, result_id in ipairs({ "program_installed", "storage_full" }) do
        reset(true, { [case.operation] = result_id },
              table.unpack(append_choices(
                  choices, "services", "more", "goodbye")))
        expect_finished()
        expect_menu(result_id == "program_installed" and
                    "wilma_program_result" or "wilma_storage_full")
        assert(count_menus("wilma_credstick") == 2)
        expect_page("later")
        assert(operations[1] == case.operation)

        reset(true, { [case.operation] = result_id },
              table.unpack(append_choices(choices, "goodbye", "goodbye")))
        expect_finished()
        expect_menu(result_id == "program_installed" and
                    "wilma_program_result" or "wilma_storage_full")
        expect_menu("wilma_services")
        assert(count_menus("wilma_credstick") == 1)
        expect_page("later")
    end

    choices = service_choices(case.choice)
    reset(true, { [case.operation] = "unavailable" }, table.unpack(choices))
    expect_boundary()

    choices = service_choices(case.choice)
    reset(true, { [case.operation] = "unrecognized" }, table.unpack(choices))
    expect_boundary()
end

print("Wilma Temmenhoff JSON service graph permutations passed")
