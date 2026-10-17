-- Captain Quinton Jaymes' Vid-Phone service tree.
-- Menu nodes and result text are imported from the ROM; this script expresses
-- their choices and asks the host only to commit the three native effects.
local CaptQuintonJaymes = {}

local SPEAKER = "vid_phone_capt_quinton_jaymes"
local PROFILE = "field.vid_phone"
local PHONE_CALL_FINISHED = "phone_call_finished"

local function menu(id, choices)
    return game.ui.menu(id, choices, SPEAKER, PROFILE)
end

local function say(event_id)
    game.ui.say(event_id, SPEAKER, PROFILE)
end

local function result_menu(menu_id)
    if menu(menu_id, { "services", "goodbye" }) == "services" then
        return "services"
    end
    return "finished"
end

local function report_transaction(service_action, result_menu_id)
    local result = game.services.call("vid_phone", service_action)
    if result == "insufficient" then
        say("purchase_insufficient")
        return "finished"
    elseif result == "out_of_stock" then
        say("purchase_out_of_stock")
        return "finished"
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    if result ~= "purchased" and result ~= "hired" then
        return "vid_phone_service_result_unresolved"
    end
    return result_menu(result_menu_id)
end

local function hollywood_information()
    if menu("capt_quinton_hollywood_intro", { "more" }) ~= "more" then
        return "services"
    end

    local topic = menu("capt_quinton_hollywood_topics", {
        "weller_prison_break",
        "disconnect",
        "return_to_services",
    })
    if topic == "weller_prison_break" then
        if menu("capt_quinton_weller_intro", { "more" }) == "more" then
            return result_menu("capt_quinton_weller_result")
        end
        return "services"
    elseif topic == "disconnect" then
        say("services_disconnected")
        return "finished"
    end
    return "services"
end

local function firepower()
    if menu("capt_quinton_firepower_intro", { "more" }) ~= "more" then
        return "services"
    end

    local offer = menu("capt_quinton_firepower_items", {
        "frag_grenades",
        "hk227_s",
        "no_thanks",
    })
    if offer == "frag_grenades" then
        return report_transaction(
            "purchase_frag_grenades", "capt_quinton_purchase_result")
    elseif offer == "hk227_s" then
        return report_transaction(
            "purchase_hk227_s", "capt_quinton_purchase_result")
    end
    return "services"
end

local function contacts()
    local choice = menu("capt_quinton_contacts_offer", {
        "pay_for_rigger_contact",
        "decline_rigger_contact",
    })
    if choice == "pay_for_rigger_contact" then
        return report_transaction(
            "hire_sharkey", "capt_quinton_sharkey_result")
    end
    return "services"
end

function CaptQuintonJaymes.services()
    while true do
        local choice = menu("capt_quinton_services", {
            "hollywood_information",
            "firepower",
            "contacts",
        })
        local outcome
        if choice == "hollywood_information" then
            outcome = hollywood_information()
        elseif choice == "firepower" then
            outcome = firepower()
        elseif choice == "contacts" then
            outcome = contacts()
        else
            return PHONE_CALL_FINISHED
        end

        if outcome == "finished" then
            return PHONE_CALL_FINISHED
        elseif outcome ~= "services" then
            return outcome
        end
    end
end

return CaptQuintonJaymes
