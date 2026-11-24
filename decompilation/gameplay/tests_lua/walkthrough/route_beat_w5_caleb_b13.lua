local E = require("enums")

local kCalebBoothX = 256
local kCalebBoothY = 1280
local kContactFrames = 120
local kContactAttempts = 4
local kDialogueWaitFrames = 120
local kDialogueInputLimit = 60

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

return {
    steps = {
        {
            id = "w5_caleb_b13_last_run",
            milestone = 13,
            expected = {
                scene = E.Scene.ICARUS_DESCENDING,
                karma_delta = { 2, 2, 2 },
                facts = { [E.StoryFact.MICHAEL_MISSION_TOLD] = true },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.MICHAEL_ALIAS_LEARNED,
                            E.StoryFact.MICHAEL_MISSION_TOLD,
                        },
                    },
                    mode = {
                        set = { E.FieldModeFlagBit.BIT_23 },
                        clear = { E.FieldModeFlagBit.BIT_5 },
                    },
                },
            },
            run = function(host)
                -- ROM Boris command 0x2B at 0x21B82 sets alias-known bit 10.
                local alias, alias_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MICHAEL_ALIAS_LEARNED, true)
                assert(alias, alias_error or
                    "Michael-alias prerequisite setup failed")
                -- Caleb handler 0x2193A clears mode bit 5 when recording b13.
                local mode, mode_error = host.set_field_mode_flag_for_setup(
                    E.FieldModeFlagBit.BIT_5, true)
                assert(mode, mode_error or
                    "b13 mode-bit prerequisite setup failed")

                local placed, placement_error = host.place_within_scene(
                    E.Scene.ICARUS_DESCENDING, kCalebBoothX, kCalebBoothY)
                assert(placed, placement_error or "Caleb booth placement failed")

                local opened = false
                for _ = 1, kContactAttempts do
                    tap(host, "UP", kContactFrames, 0)
                    local state = host.snapshot()
                    if state.dialogue_open or state.interaction_open then
                        opened = true
                        break
                    end
                end
                assert(opened, "ordinary UP did not reach Caleb's booth event")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Caleb's b13 dialogue did not open")
                assert(host.snapshot().panel_actor == "Caleb Brightmore",
                    "Icarus event 30 opened a different speaker")

                for _ = 1, kDialogueInputLimit do
                    local state = host.snapshot()
                    if not state.dialogue_open then
                        return "Caleb's live last-run dialogue recorded b13"
                    end
                    tap(host, state.panel_option_count == 3 and "C" or "A")
                end
                error("Caleb's b13 dialogue did not return to the field")
            end,
        },
    },
}
