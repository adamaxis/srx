local E = require("enums")

local kGateEventCenterX = 3328 -- Scene-9 record 230 bounds x 0x00C0..0x00DF [C]
local kGateEventCenterY = 2816 -- Scene-9 record 230 bounds y 0x00A0..0x00BF [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 40

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local bundle = E.GroupItemId.GIFT_RED_BUFFALO_WOMAN

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "seed_mission_told_prerequisite_for_first_audience",
            milestone = 17,
            expected = {
                facts = { [story.MICHAEL_MISSION_TOLD] = true },
                flags = { story = { set = { story.MICHAEL_MISSION_TOLD } } },
            },
            run = function(host)
                -- ROM Caleb handler 0x2193A sets FFFC1E story bit 13.
                local ok, problem = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MICHAEL_MISSION_TOLD, true)
                assert(ok, problem or "b13 prerequisite setup failed")
                return "seeded the ROM b13 prerequisite"
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
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SINSEARACH_VILLAGE_GATE,
                    kGateEventCenterX, kGateEventCenterY)
                assert(placed, placement_error or "Sinsearach gate placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or "Scene-9 event 0 did not open")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Sinsearach guard dialogue did not open")

                for _ = 1, kConversationInputLimit do
                    local state = host.snapshot()
                    if not state.dialogue_open then
                        return "Lady Gillian's live first-audience Yes set b17 and granted the bundle"
                    end
                    if state.page_key == "salish.sinsearach.page.guard_return_offer" then
                        tap(host, "B")
                    elseif state.page_key ==
                           "salish.sinsearach.page.ritual_first_audience_prompt" then
                        tap(host, "A")
                    elseif state.panel_option_count == 3 then
                        tap(host, "B")
                    else
                        tap(host, "A")
                    end
                end
                error("Sinsearach first-audience dialogue did not return to the field")
            end,
        },
    },
}
