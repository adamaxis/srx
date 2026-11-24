local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kTaxiTerminalX = 29952
local kTaxiTerminalY = 2816
local kFareNuyen = 100

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
            id = "w3_redmond_to_salish_taxi",
            milestone = 3,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                nuyen_delta = 0,
                facts = { [E.StoryFact.MOTEL_BILL_PAID] = true },
                flags = {
                    story = { set = { E.StoryFact.MOTEL_BILL_PAID } },
                },
                group_items = {
                    clear = { E.GroupItemId.INDIAN_LANDS_VISA },
                },
            },
            run = function(host)
                local paid_set, paid_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid_set, paid_error or
                    "ROM motel-payment taxi prerequisite setup failed")
                local reentered, reenter_error =
                    host.reenter_scene_for_setup(E.Scene.STOKER_MOTEL)
                assert(reentered, reenter_error or
                    "Scene-2 taxi setup reload failed")
                assert(not host.snapshot().opening_auto_walk and
                       not host.snapshot().dialogue_open,
                    "Scene-2 taxi setup unexpectedly opened the motel arrival")
                assert(host.grant_nuyen(kFareNuyen)) -- Setup grant: cover the ROM 100-nuyen Redmond-to-Salish fare.
                assert(host.grant_group_item(E.GroupItemId.INDIAN_LANDS_VISA)) -- Setup grant: provide the travel visa without repeating the earlier shop beat.
                -- Taxi arrival clears FFDF40 at 0x136DE; ordinary street event
                -- 31 at 0xDB36 uses the field terminal when this value is zero.
                local street_mode, mode_error =
                    host.set_field_interaction_mode_for_setup(
                        E.FieldInteractionMode.CITY_STREET_TAXI)
                assert(street_mode, mode_error or
                    "Redmond taxi street-mode setup failed")
                local placed, placement_error = host.place_within_scene(
                    E.Scene.STOKER_MOTEL, kTaxiTerminalX, kTaxiTerminalY)
                assert(placed, placement_error or
                    "Redmond taxi-terminal placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach the Redmond taxi terminal")
                tap(host, "A")
                assert(host.wait_for_taxi_selector(180),
                    "Redmond taxi terminal did not open the selector")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.REDMOND_BARRENS])
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
                            "accepted Redmond fare did not reach Salish")
                        advance(host, 90)
                        return "used Redmond's live terminal and paid the Salish fare"
                    end
                    advance(host, 1)
                end
                error("Salish cab ride page did not appear")
            end,
        },
    },
}
