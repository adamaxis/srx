local E = require("enums")
local Taxi = require("data.taxi_destinations")

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

local function ride_to_downtown(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SALISH_SHIDHE, 7424, 2304)
    assert(placed, placement_error or "Salish taxi placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or "Salish taxi terminal did not open")
    tap(host, "A")
    assert(host.wait_for_taxi_selector(180), "Salish cab map did not open")
    local moves = (Taxi.region_to_cursor[Taxi.Region.DOWNTOWN_SEATTLE] -
        Taxi.region_to_cursor[Taxi.Region.SALISH_SHIDHE]) %
        #Taxi.cursor_to_region
    for _ = 1, moves do tap(host, "LEFT") end
    tap(host, "A")
    assert(host.wait_for_dialogue(120), "Downtown fare dialogue did not open")
    tap(host, "A")
    tap(host, "A")
    for _ = 1, 120 do
        local snapshot = host.snapshot()
        if snapshot.panel_open and
            snapshot.panel_text:find("Hop in, chum.", 1, true) then
            tap(host, "A", 1, 60)
            assert(host.wait_for_scene(E.Scene.DOWNTOWN_SEATTLE, 180),
                "accepted fare did not reach Downtown Seattle")
            return
        end
        local ok, problem = host.advance(1)
        assert(ok, problem or "taxi dialogue wait failed")
    end
    error("taxi never presented its ride page")
end

local function enter_icarus(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE, 27392, 10496)
    assert(placed, placement_error or "Icarus entrance placement failed")
    local entered, movement_error = host.hold_until_modal(
        "UP", 180, E.Scene.ICARUS_DESCENDING)
    assert(entered, movement_error or "Icarus entrance did not change scene")
    assert(host.wait_for_scene(E.Scene.ICARUS_DESCENDING, 120),
        "Icarus entrance did not reach Scene 12")
    local settled, settle_error = host.advance(120)
    assert(settled, settle_error or "Icarus scene transition did not settle")
end

local function talk_to_caleb_about_michael(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.ICARUS_DESCENDING, 256, 1280)
    assert(placed, placement_error or "Caleb booth placement failed")
    local opened = false
    for _ = 1, 4 do
        tap(host, "UP", 120, 0)
        local snapshot = host.snapshot()
        if snapshot.dialogue_open or snapshot.interaction_open then
            opened = true
            break
        end
    end
    assert(opened, "continuous ordinary UP did not reach Caleb's booth event")
    assert(host.wait_for_dialogue(120), "Caleb's booth dialogue did not open")
    assert(host.snapshot().panel_actor == "Caleb Brightmore",
        "Icarus event 30 opened a different speaker")

    for _ = 1, 60 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 3 then
            tap(host, "C") -- Route choice: leave after the required mission briefing.
        else
            tap(host, "A") -- Advance text and select the sole Michael topic.
        end
    end
    error("Caleb's first-meeting dialogue did not return to the field")
end

local fact = E.StoryFact
local mode = E.FieldModeFlagBit

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "taxi_salish_to_downtown_for_w5",
            milestone = 13,
            expected = { scene = E.Scene.DOWNTOWN_SEATTLE,
                nuyen_delta = -20 },
            run = function(host)
                ride_to_downtown(host)
                return "paid the physical Salish-to-Downtown cab fare"
            end,
        },
        {
            id = "enter_icarus_descending",
            milestone = 13,
            -- Scene 12's current captured mode does not restore exactly.
            gate = false,
            expected = { scene = E.Scene.ICARUS_DESCENDING },
            run = function(host)
                enter_icarus(host)
                return "entered Icarus through Scene 0 event 53"
            end,
        },
        {
            id = "caleb_tells_michaels_last_run_b13",
            milestone = 13,
            gate = false,
            expected = {
                scene = E.Scene.ICARUS_DESCENDING,
                facts = { [fact.MICHAEL_MISSION_TOLD] = true },
                flags = {
                    story = { set = { fact.MICHAEL_MISSION_TOLD } },
                    mode = {
                        set = { mode.BIT_23 },
                        clear = { mode.BIT_5 },
                    },
                },
            },
            run = function(host)
                talk_to_caleb_about_michael(host)
                return "Caleb's live first-meeting chain set b13"
            end,
        },
    },
}
