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

local queued_choices, pages, menus, services = {}, {}, {}, {}
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
    pages, menus, services = {}, {}, {}
    service_results = results or {}
    game.state = {
        vid_phone_contacts_resolved = true,
        vid_phone_contact_0 = true,
        vid_phone_called_0 = called,
    }
end

game.ui.cursor_menu = function(menu_id, choices, selected_index,
                               speaker_id, text_profile)
    assert(menu_id == "contacts" and selected_index == 1)
    assert(speaker_id == "" and text_profile == "field.vid_phone")
    assert(#choices == 1 and choices[1] == "capt_quinton_jaymes")
    return "confirm"
end

game.ui.menu = function(menu_id, choices, speaker_id, text_profile)
    assert(speaker_id == "vid_phone_capt_quinton_jaymes")
    assert(text_profile == "field.vid_phone")
    local definition = content.menus.profiles[text_profile][menu_id]
    assert(definition ~= nil, "missing Quinton menu " .. menu_id)
    assert(script_text[definition.title_key] ~= nil,
        "missing Quinton menu title " .. tostring(definition.title_key))
    for _, choice_id in ipairs(choices) do
        local key = definition.choice_label_keys[choice_id]
        assert(key ~= nil and script_text[key] ~= nil,
            "missing Quinton label " .. menu_id .. "." .. choice_id)
    end
    menus[#menus + 1] = { id = menu_id, choices = choices }
    local selected = table.remove(queued_choices, 1)
    assert(selected ~= nil, "unexpected Quinton menu " .. menu_id)
    for _, choice_id in ipairs(choices) do
        if choice_id == selected then return selected end
    end
    error("choice " .. selected .. " is not available in " .. menu_id)
end

game.ui.say = function(event_id, speaker_id, text_profile)
    assert(text_profile == "field.vid_phone")
    local speaker = content.speakers.profiles[speaker_id]
    local event = speaker and speaker.events[event_id]
    assert(event ~= nil,
        "missing Quinton dialogue event " .. speaker_id .. "." .. event_id)
    assert(script_text[event.title_key] ~= nil,
        "missing Quinton dialogue text " .. tostring(event.title_key))
    pages[#pages + 1] = event_id
    return true
end

game.services.call = function(domain, operation)
    assert(domain == "vid_phone")
    services[#services + 1] = operation
    return service_results[operation]
end

local FieldActivities = require("field_activities")

local function expect_page(index, event_id)
    assert(pages[index] == event_id,
        "expected Quinton page " .. event_id .. " at index " .. index)
end

local function expect_menu(index, menu_id)
    assert(menus[index] ~= nil and menus[index].id == menu_id,
        "expected Quinton menu " .. menu_id .. " at index " .. index)
end

local function expect_finished(result)
    assert(result == nil,
        "Quinton call did not return to the field terminal: " ..
        tostring(type(result) == "table" and result.reason or result))
    assert(#queued_choices == 0, "Quinton graph left unused choices")
end

-- Call history is written before the first-call page, while repeat calls use
-- the repeat record. Both call roots retain the source Services/Goodbye menu.
reset(false, {}, "goodbye")
expect_finished(FieldActivities.vid_phone())
expect_menu(1, "call_capt_quinton_jaymes_first_call")
assert(game.state.vid_phone_called_0 == true and #services == 0)

reset(true, {}, "goodbye")
expect_finished(FieldActivities.vid_phone())
expect_menu(1, "call_capt_quinton_jaymes_repeat_call")

-- Hollywood's source topics include the Weller detail/result path, a
-- disconnect page, and a return to the service root.
reset(false, { purchase_frag_grenades = "purchased" },
      "services", "hollywood_information", "more",
      "weller_prison_break", "more", "services", "firepower", "more",
      "frag_grenades", "services", "hollywood_information", "more",
      "disconnect")
expect_finished(FieldActivities.vid_phone())
expect_page(1, "services_disconnected")
expect_menu(6, "capt_quinton_weller_result")
expect_menu(10, "capt_quinton_purchase_result")
assert(#services == 1 and services[1] == "purchase_frag_grenades")

reset(true, {}, "services", "hollywood_information", "more",
      "return_to_services", "hollywood_information", "more", "disconnect")
expect_finished(FieldActivities.vid_phone())
expect_page(1, "services_disconnected")
expect_menu(5, "capt_quinton_services")

-- Each transaction result selects its own page or result menu. Purchased
-- grenades/HK227 and hired Sharkey retain their source Services/Goodbye menu.
reset(false, { purchase_frag_grenades = "purchased" }, "services",
      "firepower", "more", "frag_grenades", "goodbye")
expect_finished(FieldActivities.vid_phone())
expect_menu(5, "capt_quinton_purchase_result")
assert(#services == 1 and services[1] == "purchase_frag_grenades")

reset(false, { purchase_hk227_s = "purchased" }, "services", "firepower",
      "more", "hk227_s", "goodbye")
expect_finished(FieldActivities.vid_phone())
expect_menu(5, "capt_quinton_purchase_result")
assert(#services == 1 and services[1] == "purchase_hk227_s")

reset(false, { purchase_frag_grenades = "insufficient" }, "services",
      "firepower", "more", "frag_grenades")
expect_finished(FieldActivities.vid_phone())
expect_page(1, "purchase_insufficient")

reset(false, { purchase_hk227_s = "out_of_stock" }, "services", "firepower",
      "more", "hk227_s")
expect_finished(FieldActivities.vid_phone())
expect_page(1, "purchase_out_of_stock")

reset(false, { hire_sharkey = "hired" }, "services", "contacts",
      "pay_for_rigger_contact", "goodbye")
expect_finished(FieldActivities.vid_phone())
expect_menu(4, "capt_quinton_sharkey_result")
assert(#services == 1 and services[1] == "hire_sharkey")

reset(false, {}, "services", "contacts", "decline_rigger_contact",
      "firepower", "more", "no_thanks", "hollywood_information", "more",
      "disconnect")
expect_finished(FieldActivities.vid_phone())
expect_page(1, "services_disconnected")

-- Missing call facts and native service results stay explicit boundaries.
reset(nil, {}, "goodbye")
local result = FieldActivities.vid_phone()
assert(type(result) == "table" and result.type == "source_boundary" and
       result.reason:find(
           "branch_condition_unknown:story.vid_phone_called_0", 1, true))
assert(#pages == 0 and #menus == 0)

reset(false, {}, "services", "firepower", "more", "frag_grenades")
result = FieldActivities.vid_phone()
assert(type(result) == "table" and result.type == "source_boundary" and
       result.reason:find("transaction", 1, true))
assert(#services == 1)

reset(false, {}, "services", "firepower", "more", "frag_grenades")
service_results.purchase_frag_grenades = "unrecognized_result"
result = FieldActivities.vid_phone()
assert(type(result) == "table" and result.type == "source_boundary")

print("Captain Quinton JSON call graph permutations passed")
