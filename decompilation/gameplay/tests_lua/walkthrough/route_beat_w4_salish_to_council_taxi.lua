local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kTaxiTerminalX = 7424
local kTaxiTerminalY = 2304
local kCouncilIslandFareNuyen = 75
local kTaxiDialogueWaitFrames = 900

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
    assert(ok, problem or "taxi beat frame advance failed")
end

return {
    steps = {
        {
            id = "w4_salish_to_council_island_taxi",
            milestone = 5,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                nuyen_delta = 0,
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { E.StoryFact.MOTEL_BILL_PAID } },
                },
            },
            run = function(host)
                -- ROM taxi prerequisites read the motel-paid save bit at
                -- 0x21410/0x20714; seed it to isolate this travel beat.
                local paid_set, paid_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid_set, paid_error or
                    "ROM motel-payment taxi prerequisite setup failed")

                -- Grant the Council fare from the ROM taxi record at 0xD4464
                -- (+0x1D fare byte); the live cab must deduct all 75 nuyen.
                assert(host.grant_nuyen(kCouncilIslandFareNuyen))
                local nuyen_before_ride = host.snapshot().nuyen

                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE, kTaxiTerminalX, kTaxiTerminalY)
                assert(placed, placement_error or
                    "Salish taxi terminal setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach the Salish taxi terminal")
                tap(host, "A")
                assert(host.wait_for_taxi_selector(180),
                    "Salish taxi selector did not open")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE])
                local target = assert(
                    Taxi.region_to_cursor[Taxi.Region.COUNCIL_ISLAND])
                local left_steps = (target - current) % #Taxi.cursor_to_region
                for _ = 1, left_steps do tap(host, "LEFT") end
                tap(host, "A")
                assert(host.wait_for_dialogue(120),
                    "Council Island destination dialogue did not open")
                tap(host, "A")
                tap(host, "A")

                for _ = 1, kTaxiDialogueWaitFrames do
                    local state = host.snapshot()
                    if state.panel_open and
                       state.panel_text:find("Hop in, chum.", 1, true) then
                        tap(host, "A", 1, 60)
                        assert(host.wait_for_scene(
                            E.Scene.COUNCIL_ISLAND, 180),
                            "accepted Council Island fare did not reach Scene 4")
                        assert(host.snapshot().nuyen ==
                            nuyen_before_ride - kCouncilIslandFareNuyen,
                            "live Council Island cab did not charge 75 nuyen")
                        advance(host, 90)
                        return "paid the live Salish-to-Council Island fare"
                    end
                    advance(host, 1)
                end
                error("Council Island fare page did not appear")
            end,
        },
    },
}
