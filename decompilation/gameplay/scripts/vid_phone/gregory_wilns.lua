-- Gregory Wilns' Vid-Phone services.
-- ROM record 0x10C7FA branches to this tree at 0x10C838. Commands 0x77 and
-- 0x78 dispatch to 0x22C22 (Merlin's Lore discount) and 0x22C46 (Power Focus).
local GregoryWilns = {}

local SPEAKER = "vid_phone_gregory_wilns"
local PROFILE = "field.vid_phone"
local PHONE_CALL_FINISHED = "phone_call_finished"

local function menu(id, choices)
    return game.ui.menu(id, choices, SPEAKER, PROFILE)
end

local function say(event_id)
    game.ui.say(event_id, SPEAKER, PROFILE)
end

local function goodbye()
    say("goodbye")
    return "finished"
end

local function result_menu(menu_id)
    if menu(menu_id, { "services", "goodbye" }) == "services" then
        return "services"
    end
    return goodbye()
end

local function creature_lesson()
    for _, page_id in ipairs({
        "gregory_creature_page_1",
        "gregory_creature_page_2",
        "gregory_creature_page_3",
    }) do
        if menu(page_id, { "more" }) ~= "more" then
            return "finished"
        end
    end
    if menu("gregory_creature_page_4", { "services", "goodbye" }) ==
        "services" then
        return "services"
    end
    return goodbye()
end

local function discount_offer()
    local choice = menu("gregory_discount_offer", {
        "accept_discount",
        "decline_discount",
        "talk_later",
    })
    if choice == "decline_discount" then
        return "services"
    elseif choice == "talk_later" then
        return goodbye()
    elseif choice ~= "accept_discount" then
        return "finished"
    end

    local result = game.services.call(
        "vid_phone", "purchase_merlins_lore_discount")
    if result == "insufficient" then
        say("insufficient_balance")
        return "finished"
    elseif result == "discount_enabled" then
        return result_menu("gregory_discount_result")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

local function power_focus_offer()
    local choice = menu("gregory_power_focus_offer", {
        "buy_power_focus",
        "decline_discount",
        "talk_later",
    })
    if choice == "decline_discount" then
        return "services"
    elseif choice == "talk_later" then
        return goodbye()
    elseif choice ~= "buy_power_focus" then
        return "finished"
    end

    local result = game.services.call("vid_phone", "purchase_power_focus")
    if result == "insufficient" then
        say("insufficient_balance")
        return "finished"
    elseif result == "out_of_stock" then
        say("power_focus_unavailable")
        return "finished"
    elseif result == "ineligible" then
        say("power_focus_ineligible")
        return "finished"
    elseif result == "purchased" then
        return result_menu("gregory_power_focus_result")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

function GregoryWilns.services()
    while true do
        local choice = menu("gregory_services", {
            "paranormal_creatures",
            "magic_enhancement_device",
            "corporate_discount",
        })
        local outcome
        if choice == "paranormal_creatures" then
            outcome = creature_lesson()
        elseif choice == "magic_enhancement_device" then
            outcome = power_focus_offer()
        elseif choice == "corporate_discount" then
            outcome = discount_offer()
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

return GregoryWilns
