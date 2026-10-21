-- Sharkey's passage and Smart Goggles offers.
-- ROM contact record 0x10A7F2 maps these choices to commands 0x6E-0x70.
local Sharkey = {}
local Passages = require("data.vid_phone_passages")

local SPEAKER = "vid_phone_sharkey"
local PROFILE = "field.vid_phone"
local PHONE_CALL_FINISHED = "phone_call_finished"

local function menu(id, choices)
    return game.ui.menu(id, choices, SPEAKER, PROFILE)
end

local function say(event_id)
    game.ui.say(event_id, SPEAKER, PROFILE)
end

local function passage(menu_id, passage_id)
    local choice = menu(menu_id, {
        "pay_for_passage",
        "return_to_services",
        "other_subject",
    })
    if choice == "return_to_services" then
        return "services"
    elseif choice ~= "pay_for_passage" then
        return "finished"
    end

    local action = Passages.service_action(passage_id)
    if action == nil then
        return "vid_phone_passage_data_unresolved"
    end
    local result = game.services.call("vid_phone", action)
    if result == "insufficient" then
        say("insufficient_funds")
        return "services"
    elseif result == "passage_requested" then
        say("passage_result")
        return Passages.enter_field_scene_outcome(passage_id) or {
            type = "source_boundary",
            reason = "vid_phone:passage_destination_unresolved",
        }
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    end
    return "vid_phone_service_result_unresolved"
end

local function smart_goggles()
    local choice = menu("sharkey_smart_goggles_offer", {
        "confirm_purchase",
        "return_to_services",
        "other_subject",
    })
    if choice == "return_to_services" then
        return "services"
    elseif choice ~= "confirm_purchase" then
        return "finished"
    end

    local result = game.services.call(
        "vid_phone", "purchase_smart_goggles")
    if result == "insufficient" then
        say("insufficient_funds")
    elseif result == "out_of_stock" then
        say("smart_goggles_inventory_full")
    elseif result == "purchased" then
        say("smart_goggles_result")
    elseif result == "unavailable" then
        return "vid_phone_service_state_unresolved"
    else
        return "vid_phone_service_result_unresolved"
    end
    return "services"
end

function Sharkey.services()
    while true do
        local choice = menu("sharkey_services", {
            "pass_salish",
            "pass_sinsearach",
            "smart_goggles",
        })
        local outcome
        if choice == "pass_salish" then
            outcome = passage("sharkey_salish_passage", "salish_shidhe")
        elseif choice == "pass_sinsearach" then
            outcome = passage(
                "sharkey_sinsearach_passage", "sinsearach")
        elseif choice == "smart_goggles" then
            outcome = smart_goggles()
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

return Sharkey
