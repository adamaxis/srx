local E = require("enums")

local kRedBuffaloApproachX = 0x6100
local kRedBuffaloApproachY = 0x4100
local kApproachInputFrames = 80

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "w6_red_buffalo_gift_exchange",
            milestone = 18,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                facts = { [E.StoryFact.RED_BUFFALO_WOMAN] = true },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.RITUAL_OF_BINDING,
                            E.StoryFact.RED_BUFFALO_WOMAN,
                        },
                    },
                    mode = {
                        set = { E.FieldModeFlagBit.BIT_30 },
                        clear = { E.FieldModeFlagBit.BIT_29 },
                    },
                },
                group_items = {
                    clear = { E.GroupItemId.GIFT_RED_BUFFALO_WOMAN },
                    set = { E.GroupItemId.GIFT_LADY_GILLIAN },
                },
            },
            run = function(host)
                -- The Scene-9 ritual command sets story bit 17 at 0x2196C.
                local ritual_seeded, ritual_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.RITUAL_OF_BINDING, true)
                assert(ritual_seeded, ritual_error or
                    "Ritual of Binding setup failed")

                assert(host.grant_group_item(E.GroupItemId.GIFT_RED_BUFFALO_WOMAN)) -- ROM 0x54362..0x54370 consumes this gift bit.

                -- b17's first-audience command sets mode bit 29 at 0x21902.
                local mode_seeded, mode_error =
                    host.set_field_mode_flag_for_setup(
                        E.FieldModeFlagBit.BIT_29, true)
                assert(mode_seeded, mode_error or
                    "b17 mode setup failed")

                local before = host.snapshot()
                assert(before.story_facts[E.StoryFact.RED_BUFFALO_WOMAN] == false,
                    "Red Buffalo service must be available for its first visit")
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE,
                    kRedBuffaloApproachX, kRedBuffaloApproachY)
                assert(placed, placement_error or
                    "Red Buffalo approach placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kApproachInputFrames)
                assert(opened, movement_error or
                    "Scene-6 record 194/event 1 did not open")
                assert(host.wait_for_dialogue(120),
                    "Red Buffalo Woman dialogue did not open")
                assert(host.snapshot().panel_actor == "Red Buffalo Woman",
                    "Scene-6 event 1 opened a different speaker")
                assert(host.snapshot().panel_text:find(
                    "lone AmerIndian woman", 1, true),
                    "Red Buffalo Woman's ROM introduction was not shown")

                for _ = 1, 4 do
                    if not host.snapshot().dialogue_open then return end
                    tap(host, "A")
                end
                assert(not host.snapshot().dialogue_open,
                    "Red Buffalo gift exchange did not return to Scene 6")
                return "accepted the live Red Buffalo gift for Lady Gillian"
            end,
        },
    },
}
