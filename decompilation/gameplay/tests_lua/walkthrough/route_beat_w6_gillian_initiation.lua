local E = require("enums")

local kSinsearachEventCenterX = 3328 -- Scene-9 record 230 center from ROM bounds [C]
local kSinsearachEventCenterY = 2816 -- Scene-9 record 230 center from ROM bounds [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 48

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local effect = E.FieldModeEffectFlagBit

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_gillian_initiation",
            milestone = 21,
            expected = {
                facts = {
                    [story.MICHAEL_MISSION_TOLD] = true,
                    [story.RITUAL_OF_BINDING] = true,
                    [story.RED_BUFFALO_WOMAN] = true,
                    [story.ILENE_MET] = true,
                },
                flags = {
                    story = { set = {
                        story.MICHAEL_MISSION_TOLD,
                        story.RITUAL_OF_BINDING,
                        story.RED_BUFFALO_WOMAN,
                        story.ILENE_MET,
                    } },
                    mode = { set = { mode.BIT_28, mode.BIT_31 } },
                    effect = { set = {
                        effect.ILENE_ACTIVITY_FOLLOWUP,
                        effect.BIT_3,
                    } },
                },
            },
            run = function(host)
                -- ROM Caleb handler 0x2193A sets FFFC1E bit 13.
                local ok, problem = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MICHAEL_MISSION_TOLD, true)
                assert(ok, problem or "b13 story setup failed")
                -- First audience 0x2196C, Red Buffalo 0x5435C, and Ilene
                -- activity 0x54402 set story bits 17, 18, and 19.
                for _, bit in ipairs({ E.StoryFlagBit.RITUAL_OF_BINDING,
                        E.StoryFlagBit.RED_BUFFALO_WOMAN,
                        E.StoryFlagBit.ILENE_MET }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or "b17-b19 setup failed")
                end
                -- b17 sets mode bit 28 at 0x21902; b20 gift return at
                -- 0x21D7A sets bit 31 after clearing bit 30.
                for _, bit in ipairs({ mode.BIT_28, mode.BIT_31 }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(seeded, seed_error or "b17/b20 mode setup failed")
                end
                -- b19 sets effect bit 1 at 0x54402; command 0x37 clears bit 3
                -- at 0x21E26..0x21E38, so seed it to verify that side effect.
                for _, bit in ipairs({ effect.ILENE_ACTIVITY_FOLLOWUP,
                        effect.BIT_3 }) do
                    local seeded, seed_error =
                        host.set_field_mode_effect_flag_for_setup(bit, true)
                    assert(seeded, seed_error or "b19 effect setup failed")
                end
                return "seeded the ROM b13-b20 initiation prerequisites"
            end,
        },
        {
            id = "complete_gillian_initiation_b21",
            milestone = 21,
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
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SINSEARACH_VILLAGE_GATE,
                    kSinsearachEventCenterX, kSinsearachEventCenterY)
                assert(placed, placement_error or "Sinsearach gate placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or "Scene-9 event 0 did not open")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Sinsearach guard dialogue did not open")

                local saw_initiation_page = false
                for _ = 1, kConversationInputLimit do
                    local snapshot = host.snapshot()
                    if not snapshot.dialogue_open then
                        assert(saw_initiation_page,
                            "Gillian's initiation page was not shown")
                        return "Lady Gillian's live trial Yes completed b21 and awarded two Karma"
                    end
                    if snapshot.page_key ==
                       "salish.sinsearach.page.ritual_initiation_pleased" then
                        saw_initiation_page = true
                    end
                    if snapshot.panel_actor == "Elven guard" and
                       snapshot.panel_option_count >= 2 then
                        tap(host, "B")
                    else
                        tap(host, "A")
                    end
                end
                error("Sinsearach initiation dialogue did not return to the field")
            end,
        },
    },
}
