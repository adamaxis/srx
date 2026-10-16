-- Shared map/resource activity menus. Lua owns cursor state and the action
-- selected by the player; the Qt panel only presents each yielded menu frame.
local FieldActivities = {}
local Dialogue = require("dialogue")

local function run_cursor_menu(menu_id, choices, initial_selection,
                               speaker_id, text_profile)
    local selected = math.min(initial_selection or 1, #choices)
    while true do
        local input
        if text_profile ~= nil then
            input = game.ui.cursor_menu(
                menu_id, choices, selected, speaker_id or "", text_profile)
        else
            input = game.ui.cursor_menu(menu_id, choices, selected)
        end
        if input == "up" then
            selected = (selected - 2) % #choices + 1
        elseif input == "down" then
            selected = selected % #choices + 1
        elseif input == "confirm" then
            return choices[selected], selected
        elseif input == "cancel" or input == "back" then
            return "exit", selected
        end
    end
end

local function phone_contact_choices(contacts, page)
    local choices = {}
    if page == 0 then
        local visible = math.min(#contacts, 7)
        for index = 1, visible do
            choices[#choices + 1] = contacts[index].id
        end
        if #contacts > visible then
            choices[#choices + 1] = "next_screen"
        end
    else
        for index = 8, #contacts do
            choices[#choices + 1] = contacts[index].id
        end
        -- Page two only exists when page one has a Next row. Even when the
        -- eighth contact is the sole remaining name, the ROM still appends
        -- <Prior Screen> as the last selectable row.
        choices[#choices + 1] = "prior_screen"
    end
    return choices
end

local function call_contact(contact)
    local called_fact = "vid_phone_called_" .. contact.source_index
    local first_call = contact.always_first or game.state[called_fact] ~= true
    if first_call and not contact.always_first then
        -- ROM 0xE5E2 sets the contact's call-history bit before opening its
        -- first-call dialogue. Mr. Faradouchi and Agira have the source's
        -- two explicit exceptions and always use their first record.
        game.state[called_fact] = true
    end

    local call_kind = first_call and "first_call" or "repeat_call"
    local continuation = first_call
        and contact.first_continuation
        or contact.repeat_continuation
    if continuation then
        game.ui.say(call_kind, contact.speaker_id, "field.vid_phone")
        return nil
    end

    local choice = game.ui.menu(
        "call_" .. contact.id .. "_" .. call_kind,
        { "services", "goodbye" }, contact.speaker_id, "field.vid_phone")
    if choice == "services" then
        -- The call screen is source-backed; its contact-specific service
        -- transactions remain a separate script slice.
        return "phone_services_not_converted"
    end
    return nil
end

function FieldActivities.vid_phone()
    if game.state.vid_phone_contacts_resolved ~= true then
        return "phone_contacts_flags_unresolved"
    end

    local contacts = {}
    for _, contact in ipairs(Dialogue.vid_phone_contacts) do
        if game.state["vid_phone_contact_" .. contact.source_index] == true then
            contacts[#contacts + 1] = contact
        end
    end
    if #contacts == 0 then
        game.ui.say(
            "phone_no_contacts", "field_narrator", "field.world_event")
        return nil
    end

    local page = 0
    local selected = 1
    while true do
        local choices = phone_contact_choices(contacts, page)
        local choice
        choice, selected = run_cursor_menu(
            "contacts", choices, selected, "", "field.vid_phone")
        if choice == "exit" then
            return nil
        elseif choice == "next_screen" then
            page = 1
            selected = 1
        elseif choice == "prior_screen" then
            page = 0
            selected = 1
        else
            local contact
            for _, candidate in ipairs(contacts) do
                if candidate.id == choice then
                    contact = candidate
                    break
                end
            end
            if contact ~= nil then
                local result = call_contact(contact)
                if result ~= nil then
                    return result
                end
            end
        end
        selected = math.min(selected, #phone_contact_choices(contacts, page))
    end
end

function FieldActivities.ordinary_field_terminal()
    while true do
        local choice = run_cursor_menu("ordinary", {
            "call_taxi",
            "vid_phone",
            "cyberspace",
            "exit",
        })
        if choice == "exit" then
            return nil
        elseif choice == "vid_phone" then
            local result = FieldActivities.vid_phone()
            if result ~= nil then
                return result
            end
        elseif choice == "call_taxi" then
            return "taxi_not_converted"
        elseif choice == "cyberspace" then
            return "cyberspace_not_converted"
        end
    end
end

function FieldActivities.resource_scene_main_menu()
    local choice = run_cursor_menu("resource_main", {
        "cyberspace",
        "exit",
    })

    if choice == "exit" then
        return "return_from_world_activity"
    end
    if choice == "cyberspace" then
        -- The ROM has a separate prerequisite and Matrix-entry route. Keep
        -- the source choice visible while that independent screen remains a
        -- boundary; do not turn it into a false field return.
        return "cyberspace_not_converted"
    end
    return nil
end

return FieldActivities
