local E = require("enums")

local kSpiritEyesServiceX = 27904
local kSpiritEyesServiceY = 2032
local kServiceInputFrames = 600
local kConversationInputLimit = 20

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function seed_flag(host, bit, value, name)
    local ok, problem = host.set_field_mode_flag_for_setup(bit, value)
    assert(ok, problem or ("ROM prerequisite setup failed: " .. name))
end

return {
    steps = {
        {
            id = "w4_spirit_eyes_gargoyle_horn_turn_in",
            milestone = 6,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                group_items = { clear = { E.GroupItemId.GARGOYLE_HORN } },
                facts = { [E.StoryFact.SHAMAN_ITEM4_GIVEN] = true },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.SHAMAN_MET,
                            E.StoryFact.SHAMAN_ITEM4_GIVEN,
                        },
                    },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_15,
                            E.FieldModeFlagBit.BIT_17,
                        },
                    },
                    auxiliary = {
                        set = { E.FieldAuxiliaryFlagBit.SHAMAN_RITUAL_STAGE_1 },
                    },
                },
            },
            run = function(host)
                -- ROM first contact 0x53D5A sets story bit 5 and ritual stage 0.
                local met, met_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_MET, true)
                assert(met, met_error or
                    "Spirit Eyes first-contact story setup failed")
                seed_flag(host, E.FieldModeFlagBit.BIT_14, false, "mode bit 14")
                seed_flag(host, E.FieldModeFlagBit.BIT_15, true, "mode bit 15")
                seed_flag(host, E.FieldModeFlagBit.BIT_16, true, "mode bit 16")
                local stage, stage_error = host.set_field_auxiliary_flag_for_setup(
                    E.FieldAuxiliaryFlagBit.SHAMAN_RITUAL_STAGE_0, true)
                assert(stage, stage_error or "ritual stage 0 setup failed")

                -- Grant only group item bit 4: ROM service 0x53DCA consumes it.
                assert(host.grant_group_item(E.GroupItemId.GARGOYLE_HORN))

                local placed, placement_error = host.place_within_scene(
                    E.Scene.COUNCIL_ISLAND,
                    kSpiritEyesServiceX, kSpiritEyesServiceY)
                assert(placed, placement_error or
                    "Spirit Eyes Horn-service placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "DOWN", kServiceInputFrames)
                assert(opened, movement_error or
                    "real DOWN did not open Spirit Eyes' Horn service")
                assert(host.wait_for_dialogue(120),
                    "Spirit Eyes Horn service dialogue did not open")

                for _ = 1, kConversationInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "Spirit Eyes consumed the Horn and advanced ritual stage"
                    end
                    tap(host, "A")
                end
                error("Spirit Eyes Horn-service dialogue did not finish")
            end,
        },
    },
}
