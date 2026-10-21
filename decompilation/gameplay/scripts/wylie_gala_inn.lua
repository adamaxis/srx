-- Scene 3/event 1, Wylie's Gala Inn.
-- ROM 0x1F77C selects root 0xF4A18; command EA branches on FF018F bit 2.
-- Command 0x06 charges FFF10D (50 nuyen), and 0x1F816 opens Hotel Options
-- only when the payment/free-stay path marks that value negative.
local Inn = {}

local LOCATION_ID = "penumbra_site_09_wylies_gala_inn"

local function assert_inn_context()
    assert(game.location.type == "building" and
           game.location.id == LOCATION_ID,
        "Wylie's Gala Inn prompt ran outside its linked building")
    assert(game.field.mode == 3 and game.field.event_code == 1,
        "Wylie's Gala Inn prompt requires ROM field mode/event 3/1")
end

local function payment_result()
    local result = game.services.call("building", "charge_50")
    if result == "paid" then
        game.ui.say("room_paid")
        return "hotel-options"
    elseif result == "insufficient" then
        game.ui.say("room_insufficient")
        return "field_return"
    end
    return "wylie_payment_unavailable"
end

function Inn.present_field_exit_prompt()
    assert_inn_context()

    if game.field.alternate_result > 0 then
        -- Run completion/package state is owned by the run system. This
        -- handler presents the ROM text root selected by the field exit.
        game.ui.say("package_after")
        return "field_return"
    elseif game.field.alternate_result < 0 then
        game.ui.say("package_before")
        return "field_return"
    end

    local choice = game.ui.prompt_text_key(
        "inn_service", "wylie.gala_inn.entry", { "crash", "leave" })
    if choice == "leave" then return "field_return" end
    if choice ~= "crash" then return "field_return" end

    local free_stay = game.state.runner_feature_flag_bit_2
    if free_stay == nil then
        -- Do not guess the value of FF018F in a restored save that lacks it.
        return "wylie_feature_state_unavailable"
    elseif free_stay then
        game.ui.say("free_stay")
        return "hotel-options"
    end

    local payment_choice = game.ui.prompt_text_key(
        "room_payment", "wylie.gala_inn.room", { "pay", "leave" })
    if payment_choice == "pay" then return payment_result() end
    return "field_return"
end

return Inn
