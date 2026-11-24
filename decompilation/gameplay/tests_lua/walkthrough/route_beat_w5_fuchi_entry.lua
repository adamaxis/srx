local E = require("enums")

local kFuchiEntranceX = 2304 -- Scene-3 record 97 x bounds 0x0080..0x009F [C]
local kFuchiEntranceY = 5376 -- Scene-3 record 97 y bounds 0x0140..0x015F [C]
local kEventInputFrames = 600
local kDialogueWaitFrames = 120

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "w5_fuchi_entry_b14",
            milestone = 14,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_21,
                facts = {
                    [E.StoryFact.HOSPITAL_VISITED] = true,
                    [E.StoryFact.CYBERHEART_SAFE] = false,
                },
                flags = {
                    story = { set = { E.StoryFact.HOSPITAL_VISITED } },
                },
            },
            run = function(host)
                -- Seattle General admission sets story bit 25 at ROM 0x1F8C2.
                local admitted, admitted_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.HOSPITAL_VISITED, true)
                assert(admitted, admitted_error or
                    "Hospital Visited prerequisite setup failed")
                assert(host.snapshot().story_facts[
                    E.StoryFact.CYBERHEART_SAFE] == false,
                    "Fuchi entry requires the safe reward not yet taken")

                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT, kFuchiEntranceX, kFuchiEntranceY)
                assert(placed, placement_error or
                    "Fuchi entrance placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kEventInputFrames)
                assert(opened, movement_error or
                    "real UP did not open Fuchi event 2")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Fuchi entry page did not open")
                assert(host.snapshot().panel_actor == "Narration",
                    "Fuchi entry event opened a different speaker")

                local page_seen = false
                for _ = 1, kDialogueWaitFrames do
                    local state = host.snapshot()
                    if state.panel_open and
                       state.panel_text:find("Fuchi", 1, true) then
                        page_seen = true
                        break
                    end
                    local advanced, advance_error = host.advance(1)
                    assert(advanced, advance_error or
                        "Fuchi entry page wait failed")
                end
                assert(page_seen, "Fuchi entry text was not displayed")
                tap(host, "A")
                assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_21, 180),
                    "Fuchi entry did not continue into activity 21")
                return "entered Fuchi through Scene-3 event 2 with real UP"
            end,
        },
    },
}
