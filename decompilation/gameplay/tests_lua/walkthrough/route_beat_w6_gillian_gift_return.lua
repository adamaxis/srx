local E = require("enums")

local kSinsearachEventCenterX = 3328 -- Scene-9 record 230 center from ROM bounds [C]
local kSinsearachEventCenterY = 2816 -- Scene-9 record 230 center from ROM bounds [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 48

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local effect = E.FieldModeEffectFlagBit
local gillian_gift = E.GroupItemId.GIFT_LADY_GILLIAN

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_gillian_gift_return",
            milestone = 20,
            expected = {
                facts = {
                    [story.MICHAEL_MISSION_TOLD] = true,
                    [story.RITUAL_OF_BINDING] = true,
                    [story.RED_BUFFALO_WOMAN] = true,
                    [story.ILENE_MET] = true,
                },
                group_items = { set = { gillian_gift } },
                flags = {
                    story = { set = {
                        story.MICHAEL_MISSION_TOLD,
                        story.RITUAL_OF_BINDING,
                        story.RED_BUFFALO_WOMAN,
                        story.ILENE_MET,
                    } },
                    mode = { set = { mode.BIT_28, mode.BIT_30 } },
                    effect = { set = { effect.ILENE_ACTIVITY_FOLLOWUP } },
                },
            },
            run = function(host)
                -- Caleb handler 0x2193A sets FFFC1E bit 13.
                local ok, problem = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MICHAEL_MISSION_TOLD, true)
                assert(ok, problem or "b13 story setup failed")
                -- First-audience command 0x1B at 0x2196C sets bit 17;
                -- Red Buffalo service 0x5435C sets bit 18.
                for _, bit in ipairs({ E.StoryFlagBit.RITUAL_OF_BINDING,
                        E.StoryFlagBit.RED_BUFFALO_WOMAN }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or "b17/b18 setup failed")
                end
                -- Ilene activity 0x54402 sets FFFC1E bit 19.
                local ilene, ilene_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.ILENE_MET, true)
                assert(ilene, ilene_error or "b19 story setup failed")
                -- b17 command 0x1B at 0x21902 sets mode bit 28.
                -- b18 exchange at 0x54376..0x54384 clears bit 29 and sets 30.
                for _, bit in ipairs({ mode.BIT_28, mode.BIT_30 }) do
                    local mode_seeded, mode_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(mode_seeded, mode_error or "b17/b18 mode setup failed")
                end
                -- Ilene activity 0x54402 sets effect bit 1.
                local effect_seeded, effect_error =
                    host.set_field_mode_effect_flag_for_setup(
                        effect.ILENE_ACTIVITY_FOLLOWUP, true)
                assert(effect_seeded, effect_error or "b19 effect setup failed")
                -- b18 service 0x54362..0x54370 exchanges the red gift for this item.
                assert(host.grant_group_item(gillian_gift))
                return "seeded the ROM b13-b19 prerequisites and Gillian's gift"
            end,
        },
        {
            id = "complete_gillian_gift_return_b20",
            milestone = 20,
            expected = {
                scene = E.Scene.SINSEARACH_VILLAGE_GATE,
                group_items = { clear = { gillian_gift } },
                flags = {
                    mode = { set = { mode.BIT_31 }, clear = { mode.BIT_30 } },
                },
                karma_delta = { 1, 1, 1 },
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

                for _ = 1, kConversationInputLimit do
                    local snapshot = host.snapshot()
                    if not snapshot.dialogue_open then
                        return "Lady Gillian's live gate dialogue accepted the b20 gift and awarded one Karma"
                    end
                    if snapshot.panel_actor == "Elven guard" and
                       snapshot.panel_option_count >= 2 then
                        tap(host, "B")
                    else
                        tap(host, "A")
                    end
                end
                error("Sinsearach gift-return dialogue did not return to the field")
            end,
        },
    },
}
