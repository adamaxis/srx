local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kTaxiTerminalX = 3328
local kTaxiTerminalY = 3840
local kTaxiFareNuyen = 100
local kSalishVisa = E.GroupItemId.INDIAN_LANDS_VISA

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
    assert(ok, problem or "taxi transition advance failed")
end

return {
    steps = {
        {
            id = "w3_penumbra_to_salish_taxi",
            milestone = 1,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                nuyen_delta = 0,
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { E.StoryFact.MOTEL_BILL_PAID } },
                },
                group_items = { clear = { kSalishVisa } },
            },
            run = function(host)
                local paid_set, paid_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid_set, paid_error or
                    "ROM motel-payment taxi prerequisite setup failed")
                assert(host.grant_nuyen(kTaxiFareNuyen)) -- Setup grant: ROM 0x20714 quotes the 100-nuyen Salish fare.
                assert(host.grant_group_item(kSalishVisa)) -- Setup grant: ROM 0x2076A/0x208A8 requires this visa for Salish.

                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT,
                    kTaxiTerminalX, kTaxiTerminalY)
                assert(placed, placement_error or
                    "Penumbra taxi-terminal setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach the Penumbra taxi terminal")
                tap(host, "A")
                assert(host.wait_for_taxi_selector(180),
                    "Penumbra taxi terminal did not open the selector")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.PENUMBRA_DISTRICT])
                local target = assert(
                    Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE])
                local left_steps = (target - current) % #Taxi.cursor_to_region
                for _ = 1, left_steps do tap(host, "LEFT") end
                tap(host, "A")
                assert(host.wait_for_dialogue(120),
                    "Salish destination dialogue did not open")
                tap(host, "A")
                assert(host.snapshot().dialogue_open,
                    "cabbie did not ask for fare confirmation")
                tap(host, "A")

                for _ = 1, 900 do
                    local state = host.snapshot()
                    if state.panel_open and
                       state.panel_text:find("Hop in, chum.", 1, true) then
                        tap(host, "A", 1, 60)
                        assert(host.wait_for_scene(E.Scene.SALISH_SHIDHE, 180),
                            "accepted Penumbra fare did not reach Salish")
                        advance(host, 90)
                        return "paid the live Penumbra-to-Salish fare and consumed the visa"
                    end
                    advance(host, 1)
                end
                error("Salish cab ride page did not appear")
            end,
        },
    },
}
