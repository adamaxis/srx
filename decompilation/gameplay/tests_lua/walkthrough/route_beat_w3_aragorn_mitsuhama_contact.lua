local E = require("enums")
local mode = E.FieldModeFlagBit

local kMitsuhamaEntranceX = 11520
local kMitsuhamaEntranceY = 3072
local kScene30EntryX = 0x1F00 -- ROM placement at 0x1DAFE0 [C]
local kScene30EntryY = 0x4900 -- ROM placement at 0x1DAFE0 [C]
local kElevatorLaneX = 0x3900
local kFloorSelectionCount = 4
local kNavigationFeedbackFrames = 10
local kAnimationFramesPerFloor = 17 * 8
local kBuildingTransitionVblanks = 34 -- ROM field transition cadence [C]
local kElevatorLaneY = 0x4300
local kElevatorEventY = 0x0200
local kMinimumStepRaw = 9 -- ROM 0xBDF2 cardinal movement table [C]
local kWalkPollFrames = 16
local kAragornContactX = 7462 -- 0x1D26, beside ROM actor at (0x1D00,0x4100) [C]
local kAragornContactY = 16590 -- 0x40CE, production QA contact position [C]
local kContactInputFrames = 600

local function wait_panel(host, text)
    for _ = 1, 900 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        local ok, problem = host.advance(1)
        assert(ok, problem or "Aragorn contact dialogue advance failed")
    end
    error("Aragorn contact panel did not contain " .. string.format("%q", text))
end

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Aragorn contact frame advance failed")
end

local function wait_for_elevator_selector(host, frames)
    for _ = 1, frames do
        if host.snapshot().elevator_open then return true end
        advance(host, 1)
    end
    return host.snapshot().elevator_open
end

local function source_walk_bound(raw_distance)
    return math.floor((raw_distance + kMinimumStepRaw - 1) /
        kMinimumStepRaw) + 256 -- ROM QA heading/event settling allowance [C]
end

local function walk_to(host, button, limit, poll_frames, reached, failure)
    local elapsed = 0
    while elapsed < limit do
        local chunk = math.min(poll_frames, limit - elapsed)
        local modal, problem = host.hold_until_modal(button, chunk)
        assert(not problem, problem or failure)
        assert(not modal, "unexpected modal while walking to Mitsuhama lift")
        elapsed = elapsed + chunk
        local position = host.snapshot()
        if reached(position) then return position end
    end
    error(failure)
end

local function enter_mitsuhama(host)
    -- Begin at Penumbra, satisfy the source quest gates, and use record 95
    -- (0x18AB8C) through real UP; the live entry page is accepted below.
    for _, prerequisite in ipairs({
        { E.StoryFlagBit.OWLFEATHER_ASKED, true },
        { E.StoryFlagBit.ARAGORN_FOUND, false },
    }) do
        local seeded, seed_error = host.set_story_fact_for_setup(
            prerequisite[1], prerequisite[2])
        assert(seeded, seed_error or
            "ROM Aragorn entry story prerequisite setup failed")
    end
    for _, bit in ipairs({ mode.BIT_10, mode.BIT_12 }) do
        local seeded, seed_error = host.set_field_mode_flag_for_setup(bit, true)
        assert(seeded, seed_error or
            "ROM Aragorn entry mode prerequisite setup failed")
    end
    local placed, placement_error = host.place_within_scene(
        E.Scene.PENUMBRA_DISTRICT,
        kMitsuhamaEntranceX, kMitsuhamaEntranceY)
    assert(placed, placement_error or "Mitsuhama entrance placement failed")
    local entered, movement_error = host.hold_until_modal(
        "UP", 600, E.Scene.RESOURCE_ACTIVITY_30)
    assert(entered, movement_error or
        "real UP did not enter the after-hours Mitsuhama site")
    assert(host.wait_for_dialogue(120),
        "Mitsuhama after-hours entry page did not open")
    wait_panel(host, "Hopefully Aragorn is still here")
    tap(host, "A")
    advance(host, 2 * kBuildingTransitionVblanks)
    assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_30, 180),
        "Mitsuhama entry did not reach activity 30")
    return "real Scene-3 UP and live entry dialogue reached activity 30"
end

local function ride_to_floor_four(host)
    local start = host.snapshot()
    assert(start.scene == E.Scene.RESOURCE_ACTIVITY_30 and
           start.x == kScene30EntryX and start.y == kScene30EntryY,
        "Mitsuhama entry did not leave the runner at ROM placement 0x1DAFE0")
    -- Mirror the production-input QA route: row 33 north, east to x=0x3900,
    -- then north to the lift. The 16-frame polls bound overshoot while the
    -- buttons remain real pad input.
    walk_to(host, "UP", source_walk_bound(kScene30EntryY - kElevatorLaneY),
        kWalkPollFrames,
        function(position)
            return position.y <= kElevatorLaneY
        end,
        "real UP did not reach the Scene-30 row-33 elevator lane")
    walk_to(host, "RIGHT", source_walk_bound(kElevatorLaneX - kScene30EntryX),
        8,
        function(position)
            return position.x >= kElevatorLaneX and
                   position.x < kElevatorLaneX + 0x0100 and
                   position.y >= kElevatorLaneY - 0x0100 and
                   position.y <= kElevatorLaneY + 0x00F0
        end,
        "real RIGHT did not align the Scene-30 row-33 elevator lane")
    local opened, movement_error = host.hold_until_modal(
        "UP", source_walk_bound(kElevatorLaneY - kElevatorEventY))
    assert(opened, movement_error or "real UP did not reach the Mitsuhama lift")
    assert(wait_for_elevator_selector(host, 120),
        "Scene-30 event 29 did not open the Mitsuhama elevator selector")
    for _ = 1, kFloorSelectionCount do
        tap(host, "UP")
        advance(host, kNavigationFeedbackFrames)
    end
    tap(host, "A")
    advance(host, kFloorSelectionCount * kAnimationFramesPerFloor)
    tap(host, "START")
    assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_34, 180),
        "Mitsuhama lift did not reach floor 4 / activity 34")
    return "real elevator input reached Scene 34 and installed its ROM actors"
end

return {
    steps = {
        {
            id = "enter_mitsuhama_after_hours_for_aragorn_contact",
            milestone = 4,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_30,
                flags = {
                    story = { set = { E.StoryFact.OWLFEATHER_ASKED } },
                    mode = { set = { mode.BIT_10, mode.BIT_12 } },
                },
            },
            run = function(host)
                return enter_mitsuhama(host)
            end,
        },
        {
            id = "ride_mitsuhama_elevator_to_floor_four",
            milestone = 4,
            expected = { scene = E.Scene.RESOURCE_ACTIVITY_34 },
            run = function(host)
                return ride_to_floor_four(host)
            end,
        },
        {
            id = "w3_aragorn_mitsuhama_contact",
            milestone = 4,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_34,
                panel_text_contains = "get you outta' here",
                facts = { [E.StoryFact.ARAGORN_FOUND] = true },
                flags = {
                    story = { set = { E.StoryFact.OWLFEATHER_ASKED,
                        E.StoryFact.ARAGORN_FOUND } },
                    mode = {
                        set = { mode.BIT_13 },
                        clear = { mode.BIT_10, mode.BIT_12 },
                    },
                },
            },
            run = function(host)
                -- The real Scene-30 floor-4 lift installs A8AE from ROM list
                -- 0xA8AE. Do not seed FFE180 or the actor index: the contact
                -- event itself must arm and consume that source state.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_34,
                    kAragornContactX, kAragornContactY)
                assert(placed, placement_error or
                    "Aragorn contact actor placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kContactInputFrames)
                assert(opened, movement_error or
                    "real UP did not contact Aragorn in Scene 34")
                local dialogue, dialogue_error = host.wait_for_dialogue(120)
                assert(dialogue, dialogue_error or
                    "Scene-34 Aragorn contact dialogue did not open")
                wait_panel(host, "Walking into the room")
                local pressed, press_error = host.press("A")
                assert(pressed, press_error or
                    "pad A did not advance Aragorn's contact dialogue")
                wait_panel(host, "get you outta' here")
                return "real pad A advanced Aragorn's live contact dialogue"
            end,
        },
    },
}
