-- Shared map/resource activity menus. Lua owns cursor state and the action
-- selected by the player; the Qt panel only presents each yielded menu frame.
local FieldActivities = {}
local Dialogue = require("dialogue")
local MatrixPasscodes = require("data.matrix_passcodes")
local CaptQuintonJaymes = require("vid_phone.capt_quinton_jaymes")

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

local function say_phone_contact(contact, event_id)
    game.ui.say(event_id, contact.speaker_id, "field.vid_phone")
end

local function phone_call_finished()
    -- ROM 0xE4C8 returns from the Vid-Phone routine after one contact call.
    -- The ordinary terminal then redraws its four-option menu.
    return "phone_call_finished"
end

local function call_contact(contact)
    local called_fact = "vid_phone_called_" .. contact.source_index
    local first_call = game.state[called_fact] ~= true
    if first_call and not contact.always_first then
        -- ROM 0xE5E2 sets the contact's call-history bit before opening its
        -- first-call dialogue. Faradouchi and Agira are the two exceptions:
        -- their source commands set history only after the special offer.
        game.state[called_fact] = true
    end

    local call_kind = first_call and "first_call" or "repeat_call"
    local continuation = first_call
        and contact.first_continuation
        or contact.repeat_continuation
    if continuation then
        if contact.repeat_story_event ~= nil and not first_call then
            local choice = game.ui.menu(
                "call_" .. contact.id .. "_" .. call_kind,
                { "more" }, contact.speaker_id, "field.vid_phone")
            if choice == "more" then
                say_phone_contact(contact, contact.repeat_story_event)
            end
        else
            say_phone_contact(contact, call_kind)
        end
        return phone_call_finished()
    end

    local choice = game.ui.menu(
        "call_" .. contact.id .. "_" .. call_kind,
        { "services", "goodbye" }, contact.speaker_id, "field.vid_phone")
    if choice == "services" then
        if contact.id == "capt_quinton_jaymes" then
            return CaptQuintonJaymes.services()
        end
        return "phone_services_not_converted"
    end
    return phone_call_finished()
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
                if result == "phone_call_finished" then
                    return nil
                elseif result ~= nil then
                    return result
                end
            end
        end
        selected = math.min(selected, #phone_contact_choices(contacts, page))
    end
end

local function matrix_passcode_choices(page)
    local available = {}
    for _, passcode in ipairs(MatrixPasscodes.passcodes) do
        if game.state["matrix_passcode_bit_" .. passcode.flag_bit] == true and
           (not passcode.current_target or
            game.state.matrix_current_passcode_available == true) then
            available[#available + 1] = passcode
        end
    end

    local choices = {}
    if page == 0 then
        local visible = math.min(#available, 7)
        for index = 1, visible do
            choices[#choices + 1] = available[index].id
        end
        if #available > visible then
            choices[#choices + 1] = "next_screen"
        end
    else
        for index = 8, #available do
            choices[#choices + 1] = available[index].id
        end
        choices[#choices + 1] = "prior_screen"
    end
    return available, choices
end

function FieldActivities.enter_passcode()
    if game.state.matrix_passcode_flags_resolved ~= true then
        return "matrix_passcode_flags_unresolved"
    end
    local available = matrix_passcode_choices(0)
    if #available == 0 then
        -- The ROM builds an empty list when no FFFC28 entry is known. Return
        -- to Cyberspace so the player can choose System Search instead.
        return nil
    end

    local page, selected = 0, 1
    while true do
        local _, choices = matrix_passcode_choices(page)
        local choice
        choice, selected = run_cursor_menu(
            "passcodes", choices, selected, "", "field.main_menu")
        if choice == "exit" then
            return nil
        elseif choice == "next_screen" then
            page, selected = 1, 1
        elseif choice == "prior_screen" then
            page, selected = 0, 1
        else
            for _, passcode in ipairs(MatrixPasscodes.passcodes) do
                if passcode.id == choice then
                    if passcode.current_target then
                        return "matrix_entry_current"
                    end
                    return "matrix_entry_" .. passcode.system_id
                end
            end
        end
    end
end

function FieldActivities.enter_cyberspace()
    local deck_present = game.state.cyberspace_deck_present
    if deck_present == nil then
        return "cyberspace_access_facts_unresolved"
    end
    if not deck_present then
        game.ui.say("cyberspace_no_deck", "field_narrator", "field.main_menu")
        return nil
    end

    local mpcp_functional = game.state.cyberspace_mpcp_functional
    if mpcp_functional == nil then
        return "cyberspace_access_facts_unresolved"
    end
    if not mpcp_functional then
        game.ui.say("cyberspace_no_mpcp", "field_narrator", "field.main_menu")
        return nil
    end

    local datajack_present = game.state.cyberspace_datajack_present
    if datajack_present == nil then
        return "cyberspace_access_facts_unresolved"
    end
    if not datajack_present then
        game.ui.say("cyberspace_no_datajack", "field_narrator", "field.main_menu")
        return nil
    end

    local choice = run_cursor_menu("cyberspace", {
        "enter_passcode", "system_search", "exit",
    }, 1, "", "field.main_menu")
    if choice == "enter_passcode" then
        return FieldActivities.enter_passcode()
    elseif choice == "system_search" then
        return "matrix_system_search_requested"
    end
    return nil
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
            return "taxi_selector_requested"
        elseif choice == "cyberspace" then
            local result = FieldActivities.enter_cyberspace()
            if result ~= nil then return result end
        end
    end
end

function FieldActivities.resource_scene_main_menu()
    while true do
        local choice = run_cursor_menu("resource_main", {
            "cyberspace",
            "exit",
        })
        if choice == "exit" then
            return "return_from_world_activity"
        elseif choice == "cyberspace" then
            local result = FieldActivities.enter_cyberspace()
            if result ~= nil then return result end
        end
    end
end

return FieldActivities
