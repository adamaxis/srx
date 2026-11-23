local E = require("enums")

local kSinsearachEventCenterX = 3328 -- Scene-9 record 230 center from ROM bounds [C]
local kSinsearachEventCenterY = 2816 -- Scene-9 record 230 center from ROM bounds [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local effect = E.FieldModeEffectFlagBit

local function reach_gillian_initiation(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.SINSEARACH_VILLAGE_GATE,
        kSinsearachEventCenterX, kSinsearachEventCenterY)
    assert(placed, placement_error or "Sinsearach gate placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or "Scene-9 event 0 did not open")
    assert(host.wait_for_dialogue(120), "Sinsearach guard dialogue did not open")

    local saw_initiation_page = false
    for _ = 1, 48 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then
            assert(saw_initiation_page,
                "Gillian's initiation page was not shown")
            return
        end
        if snapshot.page_key ==
           "salish.sinsearach.page.ritual_initiation_pleased" then
            saw_initiation_page = true
        end
        if snapshot.panel_actor == "Elven guard" and
           snapshot.panel_option_count >= 2 then
            tap(host, "B") -- Choose the Elders route, then decline any paid ride.
        else
            tap(host, "A") -- Advance pages and accept Gillian's trial and answers.
        end
    end
    error("Sinsearach initiation dialogue did not return to the field")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "complete_gillian_initiation_b20",
            milestone = 20,
            expected = {
                scene = E.Scene.SINSEARACH_VILLAGE_GATE,
                facts = { [story.GILLIAN_INITIATION] = true },
                flags = {
                    story = { set = { story.GILLIAN_INITIATION } },
                    mode = { clear = { mode.BIT_23, mode.BIT_28 } },
                    effect = {
                        set = { effect.BIT_2 },
                        clear = { effect.ILENE_ACTIVITY_FOLLOWUP, effect.BIT_3 },
                    },
                },
                karma_delta = { 2, 2, 2 },
            },
            run = function(host)
                local before = host.snapshot()
                assert(before.scene == E.Scene.SINSEARACH_VILLAGE_GATE,
                    "b20 requires a production-reached Scene-9 checkpoint")
                for _, prerequisite in ipairs({
                    story.MICHAEL_MISSION_TOLD,
                    story.RITUAL_OF_BINDING,
                    story.RED_BUFFALO_WOMAN,
                    story.ILENE_MET,
                }) do
                    assert(before.story_facts[prerequisite] == true,
                        "b20 predecessor fact is missing: " .. prerequisite)
                end
                assert(before.story_facts[story.GILLIAN_INITIATION] ~= true,
                    "b20 initiation is already complete")
                reach_gillian_initiation(host)
                return "Lady Gillian's live trial Yes completed the initiation and awarded two Karma"
            end,
        },
    },
}
