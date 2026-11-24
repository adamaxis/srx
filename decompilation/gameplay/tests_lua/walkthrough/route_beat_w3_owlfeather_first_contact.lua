local E = require("enums")
local mode = E.FieldModeFlagBit

local kOwlfeatherLodgeCenterX = 21760
local kOwlfeatherLodgeCenterY = 20224

return {
    steps = {
        {
            id = "w3_owlfeather_first_contact",
            milestone = 3,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                panel_actor = "David Owlfeather",
                panel_text_contains = "How can I help you this fine day?",
                facts = {
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                    [E.StoryFact.MICHAEL_ALIAS_LEARNED] = false,
                },
                flags = {
                    story = {
                        set = { E.StoryFact.TARISLAR_ASKED_SHALE },
                        clear = { E.StoryFact.MICHAEL_ALIAS_LEARNED },
                    },
                    mode = {
                        clear = {
                            mode.BIT_8,
                            mode.BIT_9,
                        },
                    },
                },
            },
            run = function(host)
                -- Scene-6 record 193 at 0x14C108, bounds 0x0540..0x055F by
                -- 0x04E0..0x04FF; setup uses its center, then real UP [C].
                -- Service 2 at 0x53B78 requires FFFC1E bit 1 set, bit 2 clear.
                for _, prerequisite in ipairs({
                    { E.StoryFlagBit.TARISLAR_ASKED_SHALE, true },
                    { E.StoryFlagBit.MICHAEL_ALIAS_LEARNED, false },
                }) do
                    local seeded, seed_error = host.set_story_fact_for_setup(
                        prerequisite[1], prerequisite[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather first-contact prerequisite setup failed")
                end
                -- Service 2 at 0x53B78 clears FFFC30 mode bits 8 and 9.
                for _, bit in ipairs({
                    mode.BIT_8,
                    mode.BIT_9,
                }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM Owlfeather effect prerequisite setup failed")
                end
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE,
                    kOwlfeatherLodgeCenterX, kOwlfeatherLodgeCenterY)
                assert(placed, placement_error or
                    "Owlfeather lodge setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open Owlfeather's first-contact service")
                assert(host.wait_for_dialogue(120),
                    "Owlfeather first-contact dialogue did not open")
                return "real UP opened Owlfeather's ROM first-contact menu"
            end,
        },
    },
}
