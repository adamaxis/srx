local E = require("enums")

local kSinsearachGateEventCenterX = 3328 -- Scene-9 record 230 bounds x 0x00C0..0x00DF [C]
local kSinsearachGateEventCenterY = 2816 -- Scene-9 record 230 bounds y 0x00A0..0x00BF [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function reach_first_audience(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SINSEARACH_VILLAGE_GATE,
        kSinsearachGateEventCenterX, kSinsearachGateEventCenterY)
    assert(placed, placement_error or "Sinsearach gate placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or "Scene-9 event 0 did not open")
    assert(host.wait_for_dialogue(120), "Sinsearach guard dialogue did not open")

    for _ = 1, 40 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.page_key == "salish.sinsearach.page.guard_return_offer" then
            tap(host, "B") -- Refuse the 1,000-nuyen lift after completing b17.
        elseif snapshot.page_key ==
               "salish.sinsearach.page.ritual_first_audience_prompt" then
            tap(host, "A") -- Accept Lady Gillian's first-audience ritual.
        elseif snapshot.panel_option_count == 3 then
            tap(host, "B") -- Choose the Elders route from the guard's live menu.
        else
            tap(host, "A") -- Advance the live welcome and Ritual explanation pages.
        end
    end
    error("Sinsearach first-audience dialogue did not return to the field")
end

local story = E.StoryFact
local bit = E.StoryFlagBit
local mode = E.FieldModeFlagBit
local bundle = E.GroupItemId.GIFT_RED_BUFFALO_WOMAN

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "setup_blocked_b13_prerequisite_for_b17",
            milestone = 0,
            expected = {
                scene = E.Scene.SINSEARACH_VILLAGE_GATE,
                facts = { [story.MICHAEL_MISSION_TOLD] = true },
                flags = { story = { set = { story.MICHAEL_MISSION_TOLD } } },
            },
            run = function(host)
                assert(host.set_story_fact_for_setup(
                    bit.MICHAEL_MISSION_TOLD, true)) -- Route setup: seed blocked b13 so the live b17 branch can run.
                assert(host.reenter_scene_for_setup(
                    E.Scene.SINSEARACH_VILLAGE_GATE)) -- Route setup: stage the gated Scene-9 conversation after unresolved travel.
                return "seeded b13 and staged the production Scene-9 gate context"
            end,
        },
        {
            id = "complete_sinsearach_first_audience_b17",
            milestone = 17,
            expected = {
                scene = E.Scene.SINSEARACH_VILLAGE_GATE,
                facts = { [story.RITUAL_OF_BINDING] = true },
                flags = {
                    story = { set = { story.RITUAL_OF_BINDING } },
                    mode = { set = { mode.BIT_28, mode.BIT_29 } },
                },
                group_items = { set = { bundle } },
            },
            run = function(host)
                assert(host.snapshot().story_facts[story.MICHAEL_MISSION_TOLD] == true,
                    "b13 setup prerequisite is missing")
                reach_first_audience(host)
                return "Lady Gillian's live first-audience Yes set b17 and granted the bundle"
            end,
        },
    },
}
