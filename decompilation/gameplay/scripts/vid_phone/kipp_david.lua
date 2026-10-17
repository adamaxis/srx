-- Kipp David's Vid-Phone services.
-- Contact record 0x10AD8A. Commands 0x71-0x73 are handlers
-- 0x22A54 (Fairlight Excalibur), 0x22A9E (Rebound), and 0x22AF4 (Degrade).
local KippDavid = {}

local SPEAKER = "vid_phone_kipp_david"
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

local function more(menu_id)
    if menu(menu_id, { "more" }) ~= "more" then
        return "finished"
    end
    return nil
end

local function matrix_information()
    if more("kipp_matrix_intro") == "finished" then
        return "finished"
    end
    if more("kipp_matrix_corps") == "finished" then
        return "finished"
    end
    return result_menu("kipp_matrix_result")
end

local function buy_program(service_id)
    local result = game.services.call("vid_phone", service_id)
    if result == "insufficient" then
        say("insufficient_funds")
        return "finished"
    elseif result == "storage_full" then
        return result_menu("kipp_storage_full")
    elseif result == "program_installed" then
        return result_menu("kipp_program_result")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

local function programs()
    if more("kipp_program_intro") == "finished" then
        return "finished"
    end
    if more("kipp_program_details") == "finished" then
        return "finished"
    end
    local choice = menu("kipp_program_offer", {
        "buy_rebound",
        "buy_degrade",
        "different_subject",
    })
    if choice == "buy_rebound" then
        return buy_program("purchase_rebound")
    elseif choice == "buy_degrade" then
        return buy_program("purchase_degrade")
    elseif choice == "different_subject" then
        return "services"
    end
    return "finished"
end

local function cyberdeck()
    local choice = menu("kipp_deck_offer", {
        "pay_for_deck",
        "different_subject",
    })
    if choice == "different_subject" then
        return "services"
    elseif choice ~= "pay_for_deck" then
        return "finished"
    end

    local result = game.services.call("vid_phone", "purchase_excalibur")
    if result == "insufficient" then
        say("insufficient_funds")
        return "finished"
    elseif result == "deck_installed" then
        say("disconnected")
        return "finished"
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

function KippDavid.services()
    while true do
        if more("kipp_intro") == "finished" then
            return PHONE_CALL_FINISHED
        end
        local choice = menu("kipp_services", {
            "matrix",
            "programs",
            "cyberdeck",
        })
        local outcome
        if choice == "matrix" then
            outcome = matrix_information()
        elseif choice == "programs" then
            outcome = programs()
        elseif choice == "cyberdeck" then
            outcome = cyberdeck()
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

return KippDavid
