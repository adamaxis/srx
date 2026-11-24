local E = require("enums")

local kMitsuhamaEntranceX = 11520
local kMitsuhamaEntranceY = 3072
local kMitsuhamaExitX = 7936
local kMitsuhamaExitY = 19712

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        local ok, problem = host.advance(1)
        assert(ok, problem or "Mitsuhama entry setup advance failed")
    end
    error("Mitsuhama entry setup panel did not contain " ..
        string.format("%q", text))
end

return {
    steps = {
        {
            id = "w3_mitsuhama_return_to_field",
            milestone = 4,
            expected = {
                scene = E.Scene.PENUMBRA_DISTRICT,
                facts = { [E.StoryFact.OWLFEATHER_ASKED] = true },
                flags = {
                    story = { set = { E.StoryFact.OWLFEATHER_ASKED } },
                    mode = { set = { E.FieldModeFlagBit.BIT_12 } },
                },
            },
            run = function(host)
                -- ROM 0x53BCC needs FFFC1E bit 2; Mortimer's answer at 0x2323A
                -- sets the mode bit that selects the Scene-3/event-3 entry.
                local story_set, story_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.OWLFEATHER_ASKED, true)
                assert(story_set, story_error or
                    "ROM Mitsuhama return setup story prerequisite failed")
                local lead_set, lead_error = host.set_field_mode_flag_for_setup(
                    E.FieldModeFlagBit.BIT_12, true)
                assert(lead_set, lead_error or
                    "ROM Mitsuhama return setup lead failed")

                -- Establish the source-linked exit return point via real ROM
                -- record 95 at 0x18AB8C; only the Scene-30 exit is the beat.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT,
                    kMitsuhamaEntranceX, kMitsuhamaEntranceY)
                assert(placed, placement_error or
                    "Mitsuhama entrance setup placement failed")
                local entered, movement_error = host.hold_until_modal("UP", 600)
                assert(entered, movement_error or
                    "real UP did not open the source Mitsuhama entrance")
                assert(host.wait_for_dialogue(120),
                    "Mitsuhama entry passage did not open")
                wait_panel(host, "Hopefully Aragorn is still here")
                tap(host, "A")
                assert(host.wait_for_scene(E.Scene.RESOURCE_ACTIVITY_30, 180),
                    "Mitsuhama entry setup did not reach activity 30")

                local exit_placed, exit_placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_30,
                    kMitsuhamaExitX, kMitsuhamaExitY)
                assert(exit_placed, exit_placement_error or
                    "Mitsuhama exit setup placement failed")
                local exited, exit_error = host.hold_until_modal("DOWN", 600)
                assert(exited, exit_error or
                    "real DOWN did not open the Scene-30 source exit")
                assert(host.wait_for_scene(E.Scene.PENUMBRA_DISTRICT, 180),
                    "Mitsuhama field exit did not restore Penumbra")
                return "real Scene-30 DOWN restored the source Penumbra map"
            end,
        },
    },
}
