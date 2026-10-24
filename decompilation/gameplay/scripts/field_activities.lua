-- Shared map/resource activity menus. Lua owns cursor state and the action
-- selected by the player; the Qt panel only presents each yielded menu frame.
local FieldActivities = {}
local Dialogue = require("dialogue")
local DialogueGraph = require("dialogue_graph")
local MatrixPasscodes = require("data.matrix_passcodes")

-- ROM 0x0DBF8 seeds FFF118 (Matrix node) and FFFBBC (system) from the active
-- resource scene before presenting its Cyberspace / Exit Main Menu.
local resource_scene_matrix_targets = {
    [21] = { system_id = 0x16, node_id = 0x0D },
    [22] = { system_id = 0x16, node_id = 0x0D },
    [23] = { system_id = 0x16, node_id = 0x06 },
    [24] = { system_id = 0x16, node_id = 0x08 },
    [25] = { system_id = 0x16, node_id = 0x0F },
    [26] = { system_id = 0x18, node_id = 0x16 },
    [27] = { system_id = 0x18, node_id = 0x03 },
    [28] = { system_id = 0x18, node_id = 0x09 },
    [29] = { system_id = 0x18, node_id = 0x1A },
    [30] = { system_id = 0x14, node_id = 0x07 },
    [31] = { system_id = 0x14, node_id = 0x02 },
    [32] = { system_id = 0x14, node_id = 0x09 },
    [33] = { system_id = 0x14, node_id = 0x18 },
    [34] = { system_id = 0x14, node_id = 0x0E },
    [35] = { system_id = 0x15, node_id = 0x03 },
    [36] = { system_id = 0x15, node_id = 0x04 },
    [37] = { system_id = 0x15, node_id = 0x08 },
    [38] = { system_id = 0x15, node_id = 0x13 },
    [39] = { system_id = 0x15, node_id = 0x14 },
    [40] = { system_id = 0x15, node_id = 0x12 },
    [41] = { system_id = 0x17, node_id = 0x15 },
    [42] = { system_id = 0x17, node_id = 0x09 },
    [43] = { system_id = 0x17, node_id = 0x0B },
}

-- Resolve legacy service functions and JSON-graph action providers while this
-- module is initialized. The contact catalog supplies graph scope or the
-- module/function binding; yielded tasks use these resolved handlers.
local phone_service_handlers = {}
local phone_service_graph_actions = {}
for _, contact in ipairs(Dialogue.vid_phone_contacts) do
    local service_module = contact.service_module
    local service_function = contact.service_function
    if type(service_module) == "string" and service_module ~= "" then
        local service_tree = require(service_module)
        if contact.service_graph_id ~= nil then
            assert(type(service_tree.actions) == "function",
                "Vid-Phone graph contact needs an action provider: " ..
                service_module)
            phone_service_graph_actions[contact.id] = service_tree.actions
        else
            assert(type(service_function) == "string" and
                   service_function ~= "",
                "Vid-Phone service function is missing: " .. contact.id)
            local handler = service_tree[service_function]
            assert(type(handler) == "function",
                "Vid-Phone contact names a missing Lua service handler: " ..
                service_module .. "." .. service_function)
            phone_service_handlers[contact.id] = handler
        end
    end
end

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

-- Typed boundary outcome; the host shows the reason and returns to the field.
local function unconverted_phone_services()
    return {
        type = "source_boundary",
        reason = "This contact's service transactions are not converted yet",
    }
end

local function phone_call_finished()
    -- ROM 0xE4C8 returns from the Vid-Phone routine after one contact call.
    -- The ordinary terminal then redraws its four-option menu.
    return "phone_call_finished"
end

local function run_contact_graph(contact)
    local action_provider = phone_service_graph_actions[contact.id]
    if type(action_provider) ~= "function" then
        return unconverted_phone_services()
    end
    local outcome = DialogueGraph.run(
        contact.service_graph_id, action_provider(contact))
    if type(outcome) ~= "table" or type(outcome.type) ~= "string" then
        return "phone_service_result_unresolved"
    end
    -- The contact graph owns one nested call; this field-activity session owns
    -- the ROM's post-call transition to the caller.
    if outcome.type == "return_to_caller" then
        return phone_call_finished()
    end
    return outcome
end

local function call_contact_services(contact)
    if contact.service_graph_scope == "services" then
        return run_contact_graph(contact)
    end
    local handler = phone_service_handlers[contact.id]
    if type(handler) ~= "function" then
        return unconverted_phone_services()
    end
    return handler()
end

local function call_contact(contact)
    if contact.service_graph_scope == "call" then
        return run_contact_graph(contact)
    end

    local called_fact = contact.call_history_fact
    local first_call = game.state[called_fact] ~= true
    if first_call and contact.call_history_policy == "before_first_call" then
        -- ROM 0xE5E2 sets the contact's call-history bit before opening its
        -- first-call dialogue. Call-scoped JSON graphs own contacts whose ROM
        -- handlers write history only after a particular dialogue choice.
        game.state[called_fact] = true
    end

    local call_kind = first_call and contact.first_call_event or
        contact.repeat_call_event
    local continuation = first_call and contact.first_continuation or
        contact.repeat_continuation
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
        return call_contact_services(contact)
    elseif choice == "goodbye" and contact.farewell_event ~= nil then
        say_phone_contact(contact, contact.farewell_event)
    end
    return phone_call_finished()
end

function FieldActivities.vid_phone()
    if game.state.vid_phone_contacts_resolved ~= true then
        return "phone_contacts_flags_unresolved"
    end

    local contacts = {}
    for _, contact in ipairs(Dialogue.vid_phone_contacts) do
        if game.state[contact.availability_fact] == true then
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

local function cyberspace_access_ready()
    local deck_present = game.state.cyberspace_deck_present
    if deck_present == nil then
        return nil, "cyberspace_access_facts_unresolved"
    end
    if not deck_present then
        game.ui.say("cyberspace_no_deck", "field_narrator", "field.main_menu")
        return false
    end

    local mpcp_functional = game.state.cyberspace_mpcp_functional
    if mpcp_functional == nil then
        return nil, "cyberspace_access_facts_unresolved"
    end
    if not mpcp_functional then
        game.ui.say("cyberspace_no_mpcp", "field_narrator", "field.main_menu")
        return false
    end

    local datajack_present = game.state.cyberspace_datajack_present
    if datajack_present == nil then
        return nil, "cyberspace_access_facts_unresolved"
    end
    if not datajack_present then
        game.ui.say("cyberspace_no_datajack", "field_narrator", "field.main_menu")
        return false
    end
    return true
end

function FieldActivities.enter_cyberspace()
    local access_ready, unresolved = cyberspace_access_ready()
    if unresolved ~= nil then return unresolved end
    if not access_ready then return nil end

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
            local target = resource_scene_matrix_targets[
                game.location.scene_id]
            if target == nil then
                return {
                    type = "source_boundary",
                    reason = "The ROM Matrix target for this resource scene is not mapped",
                }
            end
            local access_ready, unresolved = cyberspace_access_ready()
            if unresolved ~= nil then return unresolved end
            if access_ready then
                -- ROM 0x0E40C enters 0x181F8 directly with FFF118 and
                -- FFBBC already seeded. It does not open the field-terminal
                -- Enter Passcode / System Search submenu.
                return "matrix_entry_target_" .. target.system_id .. "_" ..
                    target.node_id
            end
        end
    end
end

return FieldActivities
