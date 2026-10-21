-- ROM 0x1FCF2: Council Island scene 4/event 3 has a same-map mode-exit
-- conversation. Lua owns its story-dependent option text and the choices;
-- the Qt runtime only presents the yielded dialogue operations.
local E = require("enums")
local StoryFlags = require("story_flags")

local MedicineLodge = {}

local LOCATION_ID = "council_medicine_lodge"
local TEXT_PROFILE = "council_medicine_lodge"

local function assert_route_context()
    assert(game.location.type == "building" and
           game.location.id == LOCATION_ID,
        "Medicine Lodge dialogue ran outside its linked building")
    assert(game.field.mode == 4 and game.field.event_code == 3,
        "Medicine Lodge prompt requires ROM field mode/event 4/3")
end

function MedicineLodge.present_field_exit_prompt()
    assert_route_context()

    -- ROM 0x1FD04 selects package records 0x1118E2 and 0x1118EA for
    -- negative and positive alternate results, respectively.
    if game.field.alternate_result < 0 then
        game.ui.say("package_handoff")
        return "field_return"
    elseif game.field.alternate_result > 0 then
        game.ui.say("package_payment")
        return "field_return"
    end

    local wise_one_available = StoryFlags.is_set(
        E.StoryFlag.ARAGORN_FREED_TOLD) == true
    local choices = {
        "hire_shaman",
        "browse_store",
        wise_one_available and "wise_one" or "talk_later",
    }
    local choice = game.ui.prompt_text_key(
        "medicine_lodge_service_offer",
        "council_medicine_lodge.entry.default", choices)

    if choice == "hire_shaman" then
        -- Command 0xF0 (handler 0x24C34) sets FFFC2A bit 4 before returning
        -- the Walking Bear referral response at 0x11A8FD.
        game.state.runner_information_bit_4 = true
        game.ui.say("hire_shaman_response")
        return "field_return"
    elseif choice == "browse_store" then
        -- Command 0x1E sets FFF112 and sends the caller to field service
        -- 0x56AC0 in FFF0B6 mode 3. The host service screen is dispatched by
        -- the field owner after this Lua task returns.
        return "medicine_lodge_store_requested"
    elseif choice == "wise_one" and wise_one_available then
        -- Command 0x1F queues the eight-word scene-entry profile at 0x1668A
        -- and returns the 0x1118DA text directing Joshua to the left door.
        -- The field owner applies the typed profile on the next scene entry.
        game.ui.say("wise_one_followup")
        return "medicine_lodge_wise_one_followup"
    end

    game.ui.say("farewell")
    return "field_return"
end

return MedicineLodge
