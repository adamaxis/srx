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
            id = "prepare_rom_state_for_frosty_referral",
            milestone = 22,
            expected = {
                facts = {
                    [story.MICHAEL_MISSION_TOLD] = true,
                    [story.RITUAL_OF_BINDING] = true,
                    [story.RED_BUFFALO_WOMAN] = true,
                    [story.ILENE_MET] = true,
                    [story.GILLIAN_INITIATION] = true,
                    [story.SHAMAN_TASKS_DONE] = true,
                },
                flags = {
                    story = { set = {
                        story.MICHAEL_MISSION_TOLD,
                        story.RITUAL_OF_BINDING,
                        story.RED_BUFFALO_WOMAN,
                        story.ILENE_MET,
                        story.GILLIAN_INITIATION,
                        story.SHAMAN_TASKS_DONE,
                    } },
                    mode = { set = { mode.BIT_31 } },
                    effect = { set = { effect.BIT_2 } },
                },
            },
            run = function(host)
                -- ROM bits 13/17/18/19/20 are written by 0x2193A, 0x2196C,
                -- 0x5435C, 0x54402, and the b21 command 0x37 respectively.
                for _, bit in ipairs({ E.StoryFlagBit.MICHAEL_MISSION_TOLD,
                        E.StoryFlagBit.RITUAL_OF_BINDING,
                        E.StoryFlagBit.RED_BUFFALO_WOMAN,
                        E.StoryFlagBit.ILENE_MET,
                        E.StoryFlagBit.GILLIAN_INITIATION }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM prior-quest state setup failed")
                end
                -- Final Spirit Eyes scale service 0x53EA8 sets FFFC1E bit 9.
                local shaman, shaman_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_TASKS_DONE, true)
                assert(shaman, shaman_error or
                    "ROM shaman-task prerequisite setup failed")
                -- b20 gift return at 0x21D7A leaves mode bit 31 set; b21
                -- command 0x37 at 0x21E26..0x21E38 sets effect bit 2.
                local mode_seeded, mode_error = host.set_field_mode_flag_for_setup(
                    mode.BIT_31, true)
                assert(mode_seeded, mode_error or "b20 mode setup failed")
                local effect_seeded, effect_error =
                    host.set_field_mode_effect_flag_for_setup(effect.BIT_2, true)
                assert(effect_seeded, effect_error or "b21 effect setup failed")
                return "seeded the ROM b13-b21 and shaman-task prerequisites"
            end,
        },
        {
            id = "record_frosty_referral_b22",
            milestone = 22,
            expected = {
                scene = E.Scene.SINSEARACH_VILLAGE_GATE,
                facts = { [story.FROSTY_MET] = true },
                flags = {
                    story = { set = { story.FROSTY_MET } },
                    effect = { set = { effect.FROSTY_HARLEQUIN_EXPOSITION } },
                },
                karma_delta = { 3, 3, 3 },
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
                        return "the live Frosty introduction recorded the referral and Harlequin exposition"
                    end
                    if snapshot.panel_actor == "Elven guard" and
                       snapshot.panel_option_count >= 2 then
                        tap(host, "B") -- Decline the optional paid ride.
                    else
                        tap(host, "A") -- Accept the live Frosty referral and continue.
                    end
                end
                error("Frosty referral dialogue did not return to the field")
            end,
        },
    },
}
