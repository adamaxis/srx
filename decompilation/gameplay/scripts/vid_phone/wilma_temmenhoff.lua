-- Wilma Temmenhoff's Vid-Phone services.
-- Contact record 0x10B71E. Commands 0x74/0x75 are handlers
-- 0x22B4A (Rebound) and 0x22BA0 (Degrade): 5,000 nuyen, size
-- checked as rating 3, stored as rating 3.
local WilmaTemmenhoff = {}

local SPEAKER = "vid_phone_wilma_temmenhoff"
local PROFILE = "field.vid_phone"
local PHONE_CALL_FINISHED = "phone_call_finished"

local function menu(id, choices)
    return game.ui.menu(id, choices, SPEAKER, PROFILE)
end

local function say(event_id)
    game.ui.say(event_id, SPEAKER, PROFILE)
end

local function more(menu_id)
    if menu(menu_id, { "more" }) ~= "more" then
        return "finished"
    end
    return nil
end

-- 0x10B81C, 0x10B834, and 0x10B84C send Services to the credstick
-- page 0x10B75C and Goodbye to the service menu 0x10B778.
local function services_or_menu(menu_id)
    local choice = menu(menu_id, { "services", "goodbye" })
    if choice == "services" then
        return "credstick"
    elseif choice == "goodbye" then
        return "menu"
    end
    return "finished"
end

local function matrix_information()
    if more("wilma_matrix_intro") == "finished" then
        return "finished"
    end
    if more("wilma_matrix_companies") == "finished" then
        return "finished"
    end
    if more("wilma_matrix_sleaze") == "finished" then
        return "finished"
    end
    return services_or_menu("wilma_matrix_result")
end

local function buy_program(service_id)
    local result = game.services.call("vid_phone", service_id)
    if result == "insufficient" then
        say("insufficient_funds")
        return "finished"
    elseif result == "storage_full" then
        return services_or_menu("wilma_storage_full")
    elseif result == "program_installed" then
        return services_or_menu("wilma_program_result")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

local function programs()
    if more("wilma_program_intro") == "finished" then
        return "finished"
    end
    if more("wilma_program_degrade") == "finished" then
        return "finished"
    end
    local choice = menu("wilma_program_offer", {
        "buy_rebound",
        "buy_degrade",
        "different_subject",
    })
    if choice == "buy_rebound" then
        return buy_program("purchase_wilma_rebound")
    elseif choice == "buy_degrade" then
        return buy_program("purchase_wilma_degrade")
    elseif choice == "different_subject" then
        return "menu"
    end
    return "finished"
end

local function service_menu()
    local choice = menu("wilma_services", {
        "matrix",
        "programs",
        "goodbye",
    })
    if choice == "matrix" then
        return matrix_information()
    elseif choice == "programs" then
        return programs()
    elseif choice == "goodbye" then
        say("later")
        return "finished"
    end
    return "finished"
end

function WilmaTemmenhoff.services()
    while true do
        if more("wilma_credstick") == "finished" then
            return PHONE_CALL_FINISHED
        end
        while true do
            local outcome = service_menu()
            if outcome == "finished" then
                return PHONE_CALL_FINISHED
            elseif outcome == "credstick" then
                break
            elseif outcome ~= "menu" then
                return outcome
            end
        end
    end
end

return WilmaTemmenhoff
