package.path = "../scripts/?.lua;" .. package.path

local E = require("enums")
local Taxi = require("data.taxi_destinations")
local Cabbie = require("taxi_cabbie")

local function check(condition, message)
    if not condition then error(message, 2) end
end

local function fixture(options)
    options = options or {}
    local state = {}
    state["story_flag_bit_" .. E.StoryFlag.MOTEL_BILL_PAID] =
        options.story_flag
    state["group_item_flag_bit_" .. E.GroupItemBit.INDIAN_LANDS_VISA] =
        options.visa == true
    state["group_item_flag_bit_" .. E.GroupItemBit.ORK_ARMBAND] =
        options.armband == true

    local queued_choices = options.choices or {}
    local field = {
        mode = options.field_mode or options.current_region,
        taxi_destination_region = options.destination,
    }
    local result = {
        state = state,
        field = field,
        funds = options.funds or 0,
        speeches = {},
        prompts = {},
    }
    local function choose(menu_id, text, choices)
        result.prompts[#result.prompts + 1] = {
            menu_id = menu_id,
            text = text,
            choices = choices,
        }
        local selected = table.remove(queued_choices, 1)
        check(selected ~= nil, "missing choice for " .. menu_id)
        for _, choice in ipairs(choices) do
            if choice == selected then return selected end
        end
        error("unexpected choice " .. selected .. " for " .. menu_id)
    end

    game = {
        state = state,
        field = field,
        location = {
            scene_id = options.current_scene or options.current_region,
        },
        nuyen = function() return result.funds end,
        set_nuyen = function(amount) result.funds = amount end,
        ui = {
            say = function(event_id, speaker_id, text_profile)
                result.speeches[#result.speeches + 1] = {
                    event = event_id,
                    speaker = speaker_id,
                    profile = text_profile,
                }
                return true
            end,
            prompt_text_key = function(menu_id, text_key, choices,
                                       speaker_id, text_profile)
                check(text_key == "field.taxi.cabbie.ride_offer" and
                      speaker_id == "taxi_cabbie" and
                      text_profile == "field.taxi",
                    "ride prompt lost its speaker or catalog key")
                return choose(menu_id, text_key, choices)
            end,
            prompt_text = function(menu_id, text, choices)
                return choose(menu_id, text, choices)
            end,
        },
    }
    return result
end

local function speech_was(result, event_id)
    for _, speech in ipairs(result.speeches) do
        if speech.event == event_id then return true end
    end
    return false
end

local blocked = fixture({
    destination = Taxi.Region.DOWNTOWN_SEATTLE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = false,
    funds = 80,
})
check(Cabbie.present() == nil and speech_was(blocked, "traffic_blocked") and
      #blocked.prompts == 0 and blocked.funds == 80,
    "clear story bit 0 blocks taxi service before a prompt")

local already_there = fixture({
    destination = Taxi.Region.REDMOND_BARRENS,
    field_mode = Taxi.Region.REDMOND_BARRENS,
    current_scene = Taxi.Region.DOWNTOWN_SEATTLE,
    story_flag = true,
    funds = 80,
})
check(Cabbie.present() == nil and
      speech_was(already_there, "already_there") and
      #already_there.prompts == 0,
    "current region exits through the already-there page")

local scene_is_destination_but_field_mode_is_not = fixture({
    destination = Taxi.Region.DOWNTOWN_SEATTLE,
    field_mode = Taxi.Region.REDMOND_BARRENS,
    current_scene = Taxi.Region.DOWNTOWN_SEATTLE,
    story_flag = true,
    funds = 80,
    choices = { "no" },
})
check(Cabbie.present() == nil and
      not speech_was(scene_is_destination_but_field_mode_is_not,
                     "already_there") and
      #scene_is_destination_but_field_mode_is_not.prompts == 1,
    "scene ID does not replace FFFBF8 for the already-there check")

local declined = fixture({
    destination = Taxi.Region.DOWNTOWN_SEATTLE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    funds = 80,
    choices = { "no" },
})
check(Cabbie.present() == nil and #declined.prompts == 1 and
      declined.funds == 80,
    "No ends the cabbie conversation without a ride")

local armband_ride = fixture({
    destination = Taxi.Region.DOWNTOWN_SEATTLE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    armband = true,
    funds = 0,
    choices = { "yes" },
})
check(Cabbie.present() == "taxi_ride_accepted" and
      speech_was(armband_ride, "free_ride_armband") and
      #armband_ride.prompts == 1 and armband_ride.funds == 0 and
      armband_ride.field.mode == Taxi.Region.DOWNTOWN_SEATTLE,
    "ork armband grants a free ride without asking for a fare")

local armband_state_unresolved = fixture({
    destination = Taxi.Region.DOWNTOWN_SEATTLE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    choices = { "yes" },
})
armband_state_unresolved.state["group_item_flag_bit_" ..
    E.GroupItemBit.ORK_ARMBAND] = nil
check(Cabbie.present() == "taxi_armband_state_unresolved" and
      #armband_state_unresolved.prompts == 1 and
      #armband_state_unresolved.speeches == 0,
    "unknown leader armband state does not assume a fare or free ride")

local armband_without_visa = fixture({
    destination = Taxi.Region.SALISH_SHIDHE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    armband = true,
    visa = false,
    funds = 1000,
    choices = { "yes" },
})
check(Cabbie.present() == nil and
      speech_was(armband_without_visa, "visa_required") and
      #armband_without_visa.prompts == 1 and
      armband_without_visa.funds == 1000,
    "an armband cannot bypass the Salish visa gate")

local visa_refusal = fixture({
    destination = Taxi.Region.SALISH_SHIDHE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    armband = false,
    visa = false,
    funds = 1000,
    choices = { "yes" },
})
check(Cabbie.present() == nil and speech_was(visa_refusal, "visa_required") and
      #visa_refusal.prompts == 1,
    "Salish fare is refused when the visa flag is clear")

local fare_offers = {
    { Taxi.Region.DOWNTOWN_SEATTLE, "Downtown Seattle", 20,
      "Null perspiration, chum." },
    { Taxi.Region.PUYALLUP_BARRENS, "Puyallup Barrens", 40,
      "That's a little out of my way, chummer." },
    { Taxi.Region.REDMOND_BARRENS, "Redmond Barrens", 40,
      "That's a little out of my way, chummer." },
    { Taxi.Region.PENUMBRA_DISTRICT, "Penumbra District", 20,
      "Null perspiration, chum." },
    { Taxi.Region.COUNCIL_ISLAND, "Council Island", 75,
      "That's a little out of my way, chummer." },
    { Taxi.Region.RENRAKU_ARCOLOGY, "Renraku Arcology", 20,
      "Null perspiration, chum." },
    { Taxi.Region.SALISH_SHIDHE, "Salish-Shidhe", 100,
      "That's a little out of my way, chummer." },
}
for _, expected in ipairs(fare_offers) do
    local region, name, fare, comment = table.unpack(expected)
    local offer = fixture({
        destination = region,
        current_region = region == Taxi.Region.REDMOND_BARRENS
            and Taxi.Region.PUYALLUP_BARRENS
            or Taxi.Region.REDMOND_BARRENS,
        story_flag = true,
        visa = region == Taxi.Region.SALISH_SHIDHE,
        funds = 1000,
        choices = { "yes", "changed_mind" },
    })
    check(Cabbie.present() == nil and #offer.prompts == 2,
        "fare offer appears after Yes")
    local expected_text = table.concat({
        "Ya' say ya' wanna get to",
        name .. "?",
        comment,
        "It'll cost ya' " .. tostring(fare) .. "$ to get",
        "there.  How 'bout it?",
    }, "|")
    check(offer.prompts[2].text == expected_text,
        "fare quote preserves source line boundaries for " .. name)
end
check(Taxi.fare_comment(50) == "Rough neighborhood, pal!",
    "retains the ROM's 50-nuyen fare-comment branch")

local insufficient = fixture({
    destination = Taxi.Region.COUNCIL_ISLAND,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    funds = 74,
    choices = { "yes", "sure_thing" },
})
check(Cabbie.present() == "taxi_ride_accepted" and
      speech_was(insufficient, "insufficient_funds_free_ride") and
      insufficient.funds == 74,
    "insufficient funds accept the ride without deducting the fare")

local charged = fixture({
    destination = Taxi.Region.PUYALLUP_BARRENS,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    funds = 100,
    choices = { "yes", "sure_thing" },
})
check(Cabbie.present() == "taxi_ride_accepted" and charged.funds == 60 and
      speech_was(charged, "hop_in") and
      charged.field.mode == Taxi.Region.PUYALLUP_BARRENS,
    "accepted fare is deducted before the cabbie's normal travel line")

local salish_ride = fixture({
    destination = Taxi.Region.SALISH_SHIDHE,
    current_region = Taxi.Region.REDMOND_BARRENS,
    story_flag = true,
    visa = true,
    funds = 1000,
    choices = { "yes", "sure_thing" },
})
check(Cabbie.present() == "taxi_ride_accepted" and salish_ride.funds == 900 and
      salish_ride.state["group_item_flag_bit_" ..
          E.GroupItemBit.INDIAN_LANDS_VISA] == false,
    "accepted Salish ride deducts the fare and consumes the visa")

print("taxi cabbie dialogue, gates, fares, and visa consumption passed")
