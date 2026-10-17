-- Alesandro Hobbs' Vid-Phone services.
-- ROM command IDs 0x6B/0x6C buy level-3/level-5 passkeys (0x22934/0x2296E);
-- 0x6D charges 8,000 and sets contact bit 5 (0x229A8).
local AlesandroHobbs = {}

local SPEAKER = "vid_phone_alesandro_hobbs"
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

local function report_transaction(action_id, result_id)
    local result = game.services.call("vid_phone", action_id)
    if result == "insufficient" then
        say("purchase_insufficient")
        return "finished"
    elseif result == "out_of_stock" then
        return result_menu("alesandro_out_of_stock")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    if result ~= "purchased" and result ~= "hired" then
        return "vid_phone_service_result_unresolved"
    end
    return result_menu(result_id)
end

local function security_information()
    local topic = menu("alesandro_security_topics", {
        "patrols",
        "second_security_type",
        "avoid_detection",
    })
    if topic == "patrols" then
        say("security_patrols")
        say("security_alerts")
    elseif topic == "second_security_type" then
        say("security_technological")
    elseif topic == "avoid_detection" then
        say("security_cameras")
    end
    return "services"
end

local function passkeys()
    local choice = menu("alesandro_passkey_offer", {
        "level_3_passkey",
        "level_5_passkey",
        "return_to_services",
    })
    if choice == "level_3_passkey" then
        return report_transaction(
            "purchase_maglock_passkey_level_3",
            "alesandro_purchase_result")
    elseif choice == "level_5_passkey" then
        return report_transaction(
            "purchase_maglock_passkey_level_5",
            "alesandro_purchase_result")
    end
    return "services"
end

local function official_contact()
    local choice = menu("alesandro_official_contact_confirmation", {
        "pay_for_macklemann",
        "return_to_services",
    })
    if choice == "pay_for_macklemann" then
        return report_transaction(
            "hire_macklemann", "alesandro_macklemann_result")
    end
    return "services"
end

function AlesandroHobbs.services()
    while true do
        local choice = menu("alesandro_services", {
            "corporation_information",
            "maglock_passkey",
            "seattle_official",
        })
        local outcome
        if choice == "corporation_information" then
            outcome = security_information()
        elseif choice == "maglock_passkey" then
            outcome = passkeys()
        elseif choice == "seattle_official" then
            outcome = official_contact()
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

return AlesandroHobbs
