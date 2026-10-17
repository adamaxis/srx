-- Sgt. Macklemann's Vid-Phone services.
-- Contact record 0x10BFAC. Command 0x76 is handler 0x22BF6:
-- 5,000 nuyen sets FFFC2C bit 28. It does not set contact bit 5.
local Macklemann = {}

local SPEAKER = "vid_phone_macklemann"
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

local function services_or_disconnect(menu_id)
    local choice = menu(menu_id, { "services", "goodbye" })
    if choice == "services" then
        return "services"
    elseif choice == "goodbye" then
        say("disconnected")
        return "finished"
    end
    return "finished"
end

local function corporate_security()
    if more("macklemann_security_levels") == "finished" then
        return "finished"
    end
    if more("macklemann_security_patrols") == "finished" then
        return "finished"
    end
    return services_or_disconnect("macklemann_security_pass")
end

local function avoid_detection()
    if more("macklemann_avoid_invisibility") == "finished" then
        return "finished"
    end
    if more("macklemann_avoid_cameras") == "finished" then
        return "finished"
    end
    return services_or_disconnect("macklemann_avoid_decker")
end

local function hollywood_badge()
    local choice = menu("macklemann_hollywood_offer", { "yes", "goodbye" })
    if choice == "goodbye" then
        say("disconnected")
        return "finished"
    elseif choice ~= "yes" then
        return "finished"
    end
    local result = game.services.call("vid_phone", "issue_hollywood_badge")
    if result == "insufficient" then
        say("insufficient_funds")
        return "finished"
    elseif result == "hired" then
        say("gate")
        return "finished"
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

function Macklemann.services()
    while true do
        local choice = menu("macklemann_services", {
            "security",
            "avoid",
            "hollywood",
        })
        local outcome
        if choice == "security" then
            outcome = corporate_security()
        elseif choice == "avoid" then
            outcome = avoid_detection()
        elseif choice == "hollywood" then
            outcome = hollywood_badge()
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

return Macklemann
