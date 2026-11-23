local E = require("enums")
local Taxi = require("data.taxi_destinations")

local kDowntownTaxiCenterX = 768 -- Scene-0 record 61 bounds x 0x0020..0x003F [C]
local kDowntownTaxiCenterY = 14080 -- Scene-0 record 61 bounds y 0x0360..0x037F [C]
local kFuchiEventCenterX = 2304 -- Scene-3 record 97 bounds x 0x0080..0x009F [C]
local kFuchiEventCenterY = 5376 -- Scene-3 record 97 bounds y 0x0140..0x015F [C]

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

local function wait_panel(host, text)
    for _ = 1, 120 do
        local snapshot = host.snapshot()
        if snapshot.panel_open and snapshot.panel_text:find(text, 1, true) then
            return
        end
        local ok, problem = host.advance(1)
        assert(ok, problem or "dialogue wait failed")
    end
    error("timed out waiting for dialogue text " .. string.format("%q", text))
end

local function taxi_to_penumbra(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE, kDowntownTaxiCenterX,
        kDowntownTaxiCenterY)
    assert(placed, placement_error or "Downtown taxi placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or "Downtown taxi terminal did not open")
    tap(host, "A")
    assert(host.wait_for_taxi_selector(180), "Downtown taxi selector did not open")
    local moves = (Taxi.region_to_cursor[Taxi.Region.PENUMBRA_DISTRICT] -
        Taxi.region_to_cursor[Taxi.Region.DOWNTOWN_SEATTLE]) %
        #Taxi.cursor_to_region
    for _ = 1, moves do tap(host, "LEFT") end
    tap(host, "A")
    assert(host.wait_for_dialogue(120), "Penumbra fare dialogue did not open")
    tap(host, "A")
    tap(host, "A")
    wait_panel(host, "Hop in, chum.")
    tap(host, "A", 1, 60)
    assert(host.wait_for_scene(E.Scene.PENUMBRA_DISTRICT, 180),
        "accepted cab fare did not reach Penumbra")
end

local function enter_fuchi(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.PENUMBRA_DISTRICT, kFuchiEventCenterX, kFuchiEventCenterY)
    assert(placed, placement_error or "Fuchi event placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or "Fuchi event 2 did not open")
    assert(host.wait_for_dialogue(120), "Fuchi entry page did not open")
    assert(host.snapshot().panel_actor == "Narration",
        "Fuchi event 2 opened a different speaker")
    wait_panel(host, "Fuchi")
    tap(host, "A") -- Advance the live entry page and resume the source route.
    assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_21, 180),
        "Fuchi event 2 did not continue into resource scene 21")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "grant_taxi_fare_for_fuchi_b14_entry",
            milestone = 14,
            expected = { scene = E.Scene.DOWNTOWN_SEATTLE,
                nuyen_delta = 250 },
            run = function(host)
                assert(host.grant_nuyen(250)) -- Route setup grant: cover the physical taxi fare without grinding.
                return "granted fare money for the Fuchi entry route"
            end,
        },
        {
            id = "taxi_to_penumbra_for_fuchi_b14_entry",
            milestone = 14,
            expected = { scene = E.Scene.PENUMBRA_DISTRICT,
                nuyen_delta = -20 },
            run = function(host)
                taxi_to_penumbra(host)
                return "paid the live cab fare to Penumbra"
            end,
        },
        {
            id = "enter_fuchi_for_cyberheart_b14",
            milestone = 14,
            expected = { scene = E.Scene.RESOURCE_ACTIVITY_21 },
            run = function(host)
                local snapshot = host.snapshot()
                assert(snapshot.story_facts[E.StoryFact.HOSPITAL_VISITED] == true and
                       snapshot.story_facts[E.StoryFact.CYBERHEART_SAFE] == false,
                    "b25 Fuchi entry prerequisites are missing")
                enter_fuchi(host)
                return "Fuchi's live b25 entry page routed to resource scene 21"
            end,
        },
    },
}
