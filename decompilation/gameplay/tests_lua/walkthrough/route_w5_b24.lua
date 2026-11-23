local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kDowntownTaxiX = 768 -- Scene-0 record 61 center, ROM bounds 0x0020..0x003F [C]
local kDowntownTaxiY = 14080 -- Scene-0 record 61 center, ROM bounds 0x0360..0x037F [C]
local visa = E.GroupItemId.INDIAN_LANDS_VISA

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
    assert(ok, problem or "neutral frame advance failed")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "grant_indian_lands_visa_for_b24",
            milestone = 24,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                group_items = { set = { visa } },
            },
            run = function(host)
                assert(host.grant_group_item(visa)) -- Route setup grant: provide the Salish taxi visa without grinding.
                return "granted the Salish taxi visa for b24 travel"
            end,
        },
        {
            id = "travel_to_salish_for_b24",
            milestone = 24,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                nuyen_delta = -100,
                group_items = { clear = { visa } },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE, kDowntownTaxiX, kDowntownTaxiY)
                assert(placed, placement_error or
                    "Downtown taxi terminal placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not reach the Downtown field terminal")
                tap(host, "A") -- Select Call Taxi from the physical terminal menu.
                assert(host.wait_for_taxi_selector(180),
                    "Downtown terminal did not open the taxi selector")
                advance(host, 12)

                local current = assert(
                    Taxi.region_to_cursor[Taxi.Region.DOWNTOWN_SEATTLE])
                local target = assert(
                    Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE])
                for _ = 1, (target - current) % #Taxi.cursor_to_region do
                    tap(host, "LEFT")
                end
                tap(host, "A")
                assert(host.wait_for_dialogue(120),
                    "Salish fare dialogue did not open")
                tap(host, "A")
                assert(host.snapshot().dialogue_open,
                    "cabbie did not present the fare after accepting the ride")
                tap(host, "A")
                assert(host.snapshot().dialogue_open,
                    "cabbie did not present the ride page after the fare")
                for _ = 1, 900 do
                    local snapshot = host.snapshot()
                    if snapshot.panel_open and
                       snapshot.panel_text:find("Hop in, chum.", 1, true) then
                        tap(host, "A", 1, 60)
                        assert(host.wait_for_scene(E.Scene.SALISH_SHIDHE, 180),
                            "accepted cab fare did not reach Salish-Shidhe")
                        advance(host, 90)
                        return "paid the real Downtown-to-Salish cab fare"
                    end
                    advance(host, 1)
                end
                local snapshot = host.snapshot()
                error("Salish cab never presented its ride page; actor=" ..
                    tostring(snapshot.panel_actor) .. " panel=" ..
                    string.format("%q", snapshot.panel_text) .. " choices=" ..
                    table.concat(snapshot.panel_options or {}, "|"))
            end,
        },
    },
}
