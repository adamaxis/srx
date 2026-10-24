-- Taxi service logic recovered from 0x20714, 0x2076A, and 0x208A8 [C].
local E = require("enums")
local Taxi = require("data.taxi_destinations")
local StoryFlags = require("story_flags")

local M = {}
local SPEAKER = "taxi_cabbie"
local PROFILE = "field.taxi"
local VISA_FACT = "group_item_flag_bit_" .. E.GroupItemBit.INDIAN_LANDS_VISA
local ARMBAND_FACT = "group_item_flag_bit_" .. E.GroupItemBit.ORK_ARMBAND

function M.fare_offer_text(region)
    local destination = assert(Taxi.regions[region],
        "taxi offer needs a known destination region")
    return string.format(
        "Ya' say ya' wanna get to %s? %s It'll cost ya' %d$ to get||there.  How 'bout it?",
        destination.name, Taxi.fare_comment(destination.fare),
        destination.fare)
end

local function accepted_ride(region)
    -- 0x208FE consumes the Salish visa after accepting a trip [C].
    if region == Taxi.Region.SALISH_SHIDHE then
        game.state[VISA_FACT] = false
    end
    return "taxi_ride_accepted"
end

function M.run(destination_region, current_region)
    local destination = Taxi.regions[destination_region]
    if destination == nil or type(current_region) ~= "number" then
        return "taxi_cabbie_context_unresolved"
    end

    -- FFFC1E bit 0 blocks cab service when clear; keep its existing story name [C].
    local cab_service_available = StoryFlags.is_set(
        E.StoryFlag.MOTEL_BILL_PAID)
    if cab_service_available == nil then
        return "taxi_story_flag_unresolved"
    end
    if not cab_service_available then
        game.ui.say("traffic_blocked", SPEAKER, PROFILE)
        return nil
    end
    if destination_region == current_region then
        game.ui.say("already_there", SPEAKER, PROFILE)
        return nil
    end

    local choice = game.ui.prompt_text_key(
        "ride_offer", "field.taxi.cabbie.ride_offer",
        { "yes", "no" }, SPEAKER, PROFILE)
    if choice ~= "yes" then return nil end

    local has_armband = game.state[ARMBAND_FACT] == true
    local has_visa = game.state[VISA_FACT] == true
    if game.state[ARMBAND_FACT] == nil then
        return "taxi_armband_state_unresolved"
    end
    if destination_region == Taxi.Region.SALISH_SHIDHE and
       game.state[VISA_FACT] == nil then
        return "taxi_visa_state_unresolved"
    end
    -- 0x2076A checks the leader's FF018F bit 5. The host exposes the
    -- Pocket Secretary Ork Armband as FFFC22 group-item bit 14 [C].
    if has_armband and
       (destination_region ~= Taxi.Region.SALISH_SHIDHE or has_visa) then
        game.ui.say("free_ride_armband", SPEAKER, PROFILE)
        return accepted_ride(destination_region)
    end

    -- 0x2076A and 0x208A8 both refuse Salish without FFFC22 visa bit 1 [C].
    if destination_region == Taxi.Region.SALISH_SHIDHE and not has_visa then
        game.ui.say("visa_required", SPEAKER, PROFILE)
        return nil
    end

    choice = game.ui.prompt_text(
        "fare_offer", M.fare_offer_text(destination_region),
        { "sure_thing", "changed_mind" })
    if choice ~= "sure_thing" then return nil end

    local nuyen = game.nuyen()
    if nuyen == nil then return "taxi_nuyen_unresolved" end
    if destination.fare > nuyen then
        game.ui.say("insufficient_funds_free_ride", SPEAKER, PROFILE)
        return accepted_ride(destination_region)
    end

    game.set_nuyen(nuyen - destination.fare)
    game.ui.say("hop_in", SPEAKER, PROFILE)
    return accepted_ride(destination_region)
end

function M.present(destination_region, current_region)
    destination_region = destination_region or
        game.field.taxi_destination_region
    current_region = current_region or game.location.scene_id
    return M.run(destination_region, current_region)
end

return M
