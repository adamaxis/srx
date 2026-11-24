local E = require("enums")

local kSpiritEyesServiceX = 27904
local kSpiritEyesServiceY = 2032
local kServiceInputFrames = 600
local kConversationInputLimit = 20

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function seed_mode(host, bit, value, name)
    local ok, problem = host.set_field_mode_flag_for_setup(bit, value)
    assert(ok, problem or ("ROM prerequisite setup failed: " .. name))
end

return {
    steps = {
        {
            id = "w4_spirit_eyes_hell_hound_pelt_turn_in",
            milestone = 7,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                group_items = { clear = { E.GroupItemId.HELL_HOUND_PELT } },
                facts = { [E.StoryFact.SHAMAN_PELT_GIVEN] = true },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.SHAMAN_MET,
                            E.StoryFact.SHAMAN_ITEM4_GIVEN,
                            E.StoryFact.SHAMAN_PELT_GIVEN,
                        },
                    },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_15,
                            E.FieldModeFlagBit.BIT_18,
                        },
                    },
                    auxiliary = {
                        set = { E.FieldAuxiliaryFlagBit.SHAMAN_RITUAL_STAGE_2 },
                    },
                },
            },
            run = function(host)
                -- First contact and b6 set story bits 5/6; b6 leaves ritual
                -- stage 1 for the pelt handler at ROM continuation 0x53E08.
                local met, met_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_MET, true)
                assert(met, met_error or "Shaman first-contact setup failed")
                local horn_given, horn_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_ITEM4_GIVEN, true)
                assert(horn_given, horn_error or "Horn-service setup failed")
                seed_mode(host, E.FieldModeFlagBit.BIT_15, true, "mode bit 15")
                seed_mode(host, E.FieldModeFlagBit.BIT_17, true, "mode bit 17")
                local stage, stage_error = host.set_field_auxiliary_flag_for_setup(
                    E.FieldAuxiliaryFlagBit.SHAMAN_RITUAL_STAGE_1, true)
                assert(stage, stage_error or "ritual stage 1 setup failed")

                -- Grant only group item bit 3: ROM service 0x53E08 consumes it.
                assert(host.grant_group_item(E.GroupItemId.HELL_HOUND_PELT))

                local placed, placement_error = host.place_within_scene(
                    E.Scene.COUNCIL_ISLAND,
                    kSpiritEyesServiceX, kSpiritEyesServiceY)
                assert(placed, placement_error or
                    "Spirit Eyes Pelt-service placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "DOWN", kServiceInputFrames)
                assert(opened, movement_error or
                    "real DOWN did not open Spirit Eyes' Pelt service")
                assert(host.wait_for_dialogue(120),
                    "Spirit Eyes Pelt-service dialogue did not open")

                for _ = 1, kConversationInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "Spirit Eyes consumed the pelt and advanced ritual stage"
                    end
                    tap(host, "A")
                end
                error("Spirit Eyes Pelt-service dialogue did not finish")
            end,
        },
    },
}
