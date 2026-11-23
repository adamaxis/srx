local E = require("enums")

local kCalebBoothWorldX = 256
local kCalebBoothWorldY = 1280
local kContactFrames = 120
local kContactAttempts = 4
local kDialogueWaitFrames = 120
local kChoiceFrames = 60

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

local function talk_to_caleb_about_michael(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.ICARUS_DESCENDING, kCalebBoothWorldX, kCalebBoothWorldY)
    assert(placed, placement_error or "Caleb booth placement failed")

    local opened = false
    for _ = 1, kContactAttempts do
        tap(host, "UP", kContactFrames, 0)
        local snapshot = host.snapshot()
        if snapshot.dialogue_open or snapshot.interaction_open then
            opened = true
            break
        end
    end
    assert(opened, "ordinary UP did not reach Caleb's booth event")
    assert(host.wait_for_dialogue(kDialogueWaitFrames),
        "Caleb's booth dialogue did not open")
    assert(host.snapshot().panel_actor == "Caleb Brightmore",
        "Icarus event 30 opened a different speaker")

    for _ = 1, kChoiceFrames do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 3 then
            tap(host, "C")
        else
            tap(host, "A")
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
            id = "caleb_tells_michaels_last_run_b13",
            milestone = 13,
            expected = {
                scene = E.Scene.ICARUS_DESCENDING,
                karma_delta = { 2, 2, 2 },
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
