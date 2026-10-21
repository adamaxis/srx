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
        vid_phone_contact_5 = true,
        vid_phone_called_5 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "macklemann")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_macklemann")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Macklemann menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Macklemann title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Macklemann label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Macklemann menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_macklemann")
    assert(text_profile == "field.vid_phone")
    local event = content.speakers.profiles[speaker_id].events[event_id]
    assert(event ~= nil, "missing Macklemann event " .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Macklemann text " .. tostring(event.title_key))
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
        "Macklemann graph did not return to the field terminal")
    assert(#queued_choices == 0,
        "Macklemann graph left unused choices")
end

local function expect_boundary()
    local outcome = FieldActivities.vid_phone()
    assert(type(outcome) == "table" and outcome.type == "source_boundary",
        "unresolved Hollywood badge result must remain a source boundary")
    assert(#queued_choices == 0,
        "Macklemann boundary left unused choices")
end

local function expect_menu(menu_id)
    for _, menu in ipairs(menus) do
        if menu.id == menu_id then return end
    end
    error("Macklemann graph did not present menu " .. menu_id)
end

local function expect_page(event_id)
    for _, page in ipairs(pages) do
        if page == event_id then return end
    end
    error("Macklemann graph did not present page " .. event_id)
end

-- The shared caller still selects first/repeat copy and writes call history.
reset(false, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_macklemann_first_call")
assert(game.state.vid_phone_called_5 == true and #operations == 0)

reset(true, {}, "goodbye")
expect_finished()
assert(menus[1].id == "call_macklemann_repeat_call")

-- Both information trees preserve Services loops. Hollywood Goodbye disconnects.
reset(true, {},
      "services",
      "security", "more", "more", "services",
      "avoid", "more", "more", "services",
      "hollywood", "goodbye")
expect_finished()
for _, menu_id in ipairs({
    "macklemann_security_levels", "macklemann_security_patrols",
    "macklemann_security_pass", "macklemann_avoid_invisibility",
    "macklemann_avoid_cameras", "macklemann_avoid_decker",
    "macklemann_hollywood_offer",
}) do
    expect_menu(menu_id)
end
expect_page("disconnected")

-- Each information result's Goodbye edge disconnects directly.
reset(true, {}, "services", "security", "more", "more", "goodbye")
expect_finished()
expect_page("disconnected")

reset(true, {}, "services", "avoid", "more", "more", "goodbye")
expect_finished()
expect_page("disconnected")

-- The Hollywood offer has a separate Goodbye edge and one typed transaction.
reset(true, {}, "services", "hollywood", "goodbye")
expect_finished()
expect_page("disconnected")
assert(#operations == 0)

for _, case in ipairs({
    { result = "hired", page = "gate" },
    { result = "insufficient", page = "insufficient_funds" },
}) do
    reset(true, { issue_hollywood_badge = case.result },
          "services", "hollywood", "yes")
    expect_finished()
    expect_page(case.page)
    assert(#operations == 1 and
           operations[1] == "issue_hollywood_badge")
end

reset(true, { issue_hollywood_badge = "unavailable" },
      "services", "hollywood", "yes")
expect_boundary()

reset(true, { issue_hollywood_badge = "unrecognized" },
      "services", "hollywood", "yes")
expect_boundary()

print("Sgt. Macklemann JSON service graph permutations passed")
