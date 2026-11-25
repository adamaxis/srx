local E = require("enums")

local kLairEventCenterX = 256 -- Scene-47 record 62, x=0..32 [C: 0x1D54B0]
local kLairEventCenterY = 9984 -- Scene-47 origin y=608 plus record y=0..32 [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 32

local story = E.StoryFact
local item = E.GroupItemId.FEATHERED_SCALE

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_context_for_lair_event",
            milestone = 8,
            expected = {
                scene = E.Scene.FEATHERED_SERPENT_LAIR,
                facts = {
                    [story.SHAMAN_PELT_GIVEN] = true,
                    [story.SERPENT_SCALE_TAKEN] = false,
                },
                flags = { story = { set = { story.SHAMAN_PELT_GIVEN } } },
                group_items = { clear = { item } },
            },
            run = function(host)
                -- Event 25 at 0x1D54B0 reaches 0x53EFA only with FFFC10=0x13.
                -- Seed that ROM transition context plus b7; leave b8/item clear.
                local pelt, pelt_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_PELT_GIVEN, true)
                assert(pelt, pelt_error or "b7 prerequisite setup failed")
                local scale, scale_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SERPENT_SCALE_TAKEN, false)
                assert(scale, scale_error or "b8 clear-state setup failed")
                local context, context_error =
                    host.set_field_transition_context_for_setup(
                        E.FieldTransitionContext.SERPENT_LAIR_ENCOUNTER)
                assert(context, context_error or
                    "ROM lair transition-context setup failed")
                local field_mode, field_mode_error =
                    host.set_field_interaction_mode_for_setup(
                        E.FieldMode.SERPENT_LAIR_ENCOUNTER)
                assert(field_mode, field_mode_error or
                    "ROM lair field-mode setup failed")
                local resource_mode, resource_mode_error =
                    host.set_field_resource_mode_for_setup(
                        E.FieldMode.SERPENT_LAIR_ENCOUNTER)
                assert(resource_mode, resource_mode_error or
                    "ROM lair resource-mode setup failed")
                return "seeded b7, FFFBF8=8, and FFFC10=0x13; b8/item remain clear"
            end,
        },
        {
            id = "complete_serpent_scale_encounter_b8",
            milestone = 8,
            expected = {
                scene = E.Scene.FEATHERED_SERPENT_LAIR,
                facts = { [story.SERPENT_SCALE_TAKEN] = true },
                flags = { story = { set = { story.SERPENT_SCALE_TAKEN } } },
                group_items = { set = { item } },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.FEATHERED_SERPENT_LAIR,
                    kLairEventCenterX, kLairEventCenterY)
                assert(placed, placement_error or "lair event placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not open Scene-47 event 25")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Licourtrix dialogue did not open")

                local saw_licourtrix = false
                for _ = 1, kConversationInputLimit do
                    local snapshot = host.snapshot()
                    if not snapshot.dialogue_open then
                        assert(saw_licourtrix,
                            "Licourtrix encounter was not shown")
                        return "live event-25 dialogue granted b8 and Feathered Scale"
                    end
                    saw_licourtrix = saw_licourtrix or
                        snapshot.panel_actor == "Licourtrix"
                    if snapshot.panel_option_count == 2 then
                        -- ROM response B selects the truthful scale answer.
                        tap(host, "B")
                    else
                        tap(host, "A")
                    end
                end
                error("Licourtrix encounter did not return to Scene 47")
            end,
        },
    },
}
