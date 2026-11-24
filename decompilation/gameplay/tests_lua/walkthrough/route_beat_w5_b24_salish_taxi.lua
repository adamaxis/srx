local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kDowntownTaxiX = 768
local kDowntownTaxiY = 14080
local kSalishFareNuyen = 100
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kTaxiPageWaitFrames = 900

local function tap(host, button, hold_frames, settle_frames)
    local ok, problem
    if hold_frames == nil then
        ok, problem = host.press(button)
    elseif settle_frames == nil then
        ok, problem = host.press(button, hold_frames)
    else
        ok, problem = host.press(button, hold_frames, settle_frames)
    end
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Salish taxi beat advance failed")
end

return {
    steps = {
        {
            id = "w5_b24_downtown_to_salish_taxi",
            milestone = 24,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                nuyen_delta = 0,
                group_items = { clear = { E.GroupItemId.INDIAN_LANDS_VISA } },
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { E.StoryFact.MOTEL_BILL_PAID } },
                },
            },
            run = function(host)
                -- ROM taxi service checks motel-paid story bit 0 at
                -- 0x21410/0x20714 before accepting the ride.
                local paid, paid_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid, paid_error or
                    "motel-paid taxi prerequisite setup failed")

                -- Grant the 100-nuyen Salish fare from taxi record 0xD4464+0x1D.
                assert(host.grant_nuyen(kSalishFareNuyen))
                -- Grant the visa required by the Salish gate at ROM 0x2076A.
                assert(host.grant_group_item(E.GroupItemId.INDIAN_LANDS_VISA))
                local nuyen_before_ride = host.snapshot().nuyen

                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE, kDowntownTaxiX, kDowntownTaxiY)
                assert(placed, placement_error or
                    "Downtown taxi terminal placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not open the Downtown taxi terminal")
                tap(host, "A")
                assert(host.wait_for_taxi_selector(180),
                    "Downtown taxi selector did not open")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.DOWNTOWN_SEATTLE])
                local target = assert(
                    Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE])
                for _ = 1, (target - current) % #Taxi.cursor_to_region do
                    tap(host, "LEFT")
                end
                tap(host, "A")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Salish fare dialogue did not open")
                tap(host, "A")
                tap(host, "A")

                for _ = 1, kTaxiPageWaitFrames do
                    local state = host.snapshot()
                    if state.panel_open and
                       state.panel_text:find("Hop in, chum.", 1, true) then
                        tap(host, "A", 1, 60)
                        assert(host.wait_for_scene(E.Scene.SALISH_SHIDHE, 180),
                            "accepted fare did not reach Salish-Shidhe")
                        local after = host.snapshot()
                        assert(after.nuyen == nuyen_before_ride -
                            kSalishFareNuyen,
                            "live Salish taxi did not charge 100 nuyen")
                        advance(host, 90)
                        return "paid the live Downtown-to-Salish fare"
                    end
                    advance(host, 1)
                end
                error("Salish taxi ride page did not appear")
            end,
        },
    },
}
