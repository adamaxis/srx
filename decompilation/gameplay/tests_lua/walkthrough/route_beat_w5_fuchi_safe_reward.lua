local E = require("enums")

local kFuchiEntranceX = 2304
local kFuchiEntranceY = 5376
local kProjectedCellApproachOffset = 512
local kEntryInputFrames = 600
local kDialogueWaitFrames = 120
local kSafeInputFrames = 120
local kDialogueInputLimit = 20

-- Scene-21 records 124-130 at ROM 0x1D7A3E..0x1D7A7A are projected-terrain
-- event 0x1A cells routed through 0x16760 and safe handler 0x20C90.
local kSafeEventCenters = {
    { x = 18688, y = 3328, record = 124 },
    { x = 17152, y = 6912, record = 125 },
    { x = 17152, y = 9984, record = 126 },
    { x = 17152, y = 13056, record = 127 },
    { x = 9472, y = 768, record = 128 },
    { x = 1792, y = 768, record = 129 },
    { x = 1792, y = 19200, record = 130 },
}

local kCellApproaches = {
    { x = -kProjectedCellApproachOffset, y = 0, direction = "RIGHT" },
    { x = 0, y = kProjectedCellApproachOffset, direction = "UP" },
}

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Fuchi safe beat frame advance failed")
end

local function enter_fuchi(host)
    -- ROM 0x1F8C2 sets Hospital Visited bit 25; title service 14 at 0x54152
    -- uses that prerequisite and 0x5418C arms the safe before activity 21.
    local admitted, admitted_error = host.set_story_fact_for_setup(
        E.StoryFlagBit.HOSPITAL_VISITED, true)
    assert(admitted, admitted_error or
        "Hospital Visited prerequisite setup failed")

    local placed, placement_error = host.place_within_scene(
        E.Scene.PENUMBRA_DISTRICT, kFuchiEntranceX, kFuchiEntranceY)
    assert(placed, placement_error or "Fuchi entrance placement failed")
    local opened, movement_error = host.hold_until_modal("UP", kEntryInputFrames)
    assert(opened, movement_error or "real UP did not open Fuchi entry")
    assert(host.wait_for_dialogue(kDialogueWaitFrames),
        "Fuchi entry dialogue did not open")
    assert(host.snapshot().panel_actor == "Narration",
        "Fuchi entry opened a different speaker")
    tap(host, "A")
    assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_21, 180),
        "Fuchi entry did not arm and load activity 21")
end

local function find_safe_reward(host)
    for _, center in ipairs(kSafeEventCenters) do
        for _, approach in ipairs(kCellApproaches) do
            local placed, placement_error = host.place_within_scene(
                E.Scene.RESOURCE_ACTIVITY_21,
                center.x + approach.x, center.y + approach.y)
            if placed then
                -- These are all ROM record 124-130 event-0x1A cells; only
                -- ordinary pad movement can publish the projected event.
                host.hold_until_modal(approach.direction, kSafeInputFrames)
                local state = host.snapshot()
                if state.story_facts[E.StoryFact.CYBERHEART_SAFE] == true then
                    return center.record
                end
            elseif placement_error then
                -- A rejected in-room approach is skipped; it does not change
                -- the tested room or seed any gameplay result.
            end
        end
    end
    error("Fuchi safe reward did not trigger at Scene-21 records 124-130")
end

return {
    steps = {
        {
            id = "w5_fuchi_prototype_cyberheart_safe_reward",
            milestone = 14,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_21,
                group_items = { set = { E.GroupItemId.PROTOTYPE_CYBERHEART } },
                facts = {
                    [E.StoryFact.HOSPITAL_VISITED] = true,
                    [E.StoryFact.CYBERHEART_SAFE] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.HOSPITAL_VISITED,
                            E.StoryFact.CYBERHEART_SAFE,
                        },
                    },
                    effect = {
                        set = {
                            E.FieldModeEffectFlagBit.CYBERHEART_CARRIED_TIP,
                        },
                    },
                },
            },
            run = function(host)
                enter_fuchi(host)
                local record = find_safe_reward(host)
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Cyberheart safe reward dialogue did not open")
                for _ = 1, kDialogueInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "collected the Cyberheart from Scene-21 record " ..
                            tostring(record)
                    end
                    tap(host, "A")
                end
                error("Cyberheart safe reward dialogue did not finish")
            end,
        },
    },
}
