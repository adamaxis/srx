local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kTaxiTerminalX = 7424
local kTaxiTerminalY = 2304
local kDowntownFareNuyen = 20

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
    assert(ok, problem or "Salish taxi advance failed")
end

return {
    steps = {
        {
            id = "w5_salish_to_downtown_taxi",
            milestone = 13,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                nuyen_delta = 0,
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = { story = { set = { E.StoryFact.MOTEL_BILL_PAID } } },
            },
            run = function(host)
                -- ROM taxi availability at 0x20714 requires FFFC1E bit 0.
                local paid, paid_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid, paid_error or
                    "ROM Salish taxi prerequisite setup failed")
                -- Setup grant: provide the ROM Downtown fare (20 nuyen,
                -- taxi record byte +0x1D in the table at 0xD4464).
                assert(host.grant_nuyen(kDowntownFareNuyen))
                local nuyen_before_ride = host.snapshot().nuyen

                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE, kTaxiTerminalX, kTaxiTerminalY)
                assert(placed, placement_error or
                    "Salish taxi setup placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach the Salish taxi terminal")
                tap(host, "A")
                assert(host.wait_for_taxi_selector(180),
                    "Salish taxi selector did not open")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE])
                local target = assert(
                    Taxi.region_to_cursor[Taxi.Region.DOWNTOWN_SEATTLE])
                local left_steps =
                    (target - current) % #Taxi.cursor_to_region
                for _ = 1, left_steps do tap(host, "LEFT") end
                tap(host, "A")
                assert(host.wait_for_dialogue(120),
                    "Downtown destination dialogue did not open")
                tap(host, "A")
                assert(host.snapshot().dialogue_open,
                    "cabbie did not ask for fare confirmation")
                tap(host, "A")

                for _ = 1, 900 do
                    local state = host.snapshot()
                    if state.panel_open and
                       state.panel_text:find("Hop in, chum.", 1, true) then
                        tap(host, "A", 1, 60)
                        assert(host.wait_for_scene(
                            E.Scene.DOWNTOWN_SEATTLE, 180),
                            "accepted Downtown fare did not reach Scene 0")
                        assert(host.snapshot().nuyen ==
                            nuyen_before_ride - kDowntownFareNuyen,
                            "live Salish-to-Downtown taxi did not charge 20 nuyen")
                        advance(host, 90)
                        return "paid the live Salish-to-Downtown cab fare"
                    end
                    advance(host, 1)
                end
                error("Downtown taxi fare page did not appear")
            end,
        },
    },
}
