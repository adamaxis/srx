local E = require("enums")

local kIleneWorldX = 3712 -- ROM actor route 0xA8E4 positions Ilene at x=0x0E80 [C]
local kIleneWorldY = 1792 -- ROM actor route 0xA8E4 positions Ilene at y=0x0700 [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local story = E.StoryFact
local bit = E.StoryFlagBit
local mode = E.FieldModeFlagBit
local effect = E.FieldModeEffectFlagBit
local gillian_gift = E.GroupItemId.GIFT_LADY_GILLIAN

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "setup_b18_prerequisites_for_b19",
            milestone = 0,
            expected = {
                scene = E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR,
                facts = { [story.RED_BUFFALO_WOMAN] = true },
                flags = {
                    story = { set = { story.RED_BUFFALO_WOMAN } },
                    mode = { set = { mode.BIT_31 } },
                    effect = { set = { effect.ILENE_INFORMATION_HEARD } },
                },
                group_items = { set = { gillian_gift } },
            },
            run = function(host)
                assert(host.set_story_fact_for_setup(
                    bit.RED_BUFFALO_WOMAN, true)) -- Route setup: seed blocked b18's story fact.
                assert(host.grant_group_item(gillian_gift)) -- Route setup grant: provide Lady Gillian's b18 gift.
                assert(host.set_field_mode_flag_for_setup(mode.BIT_31, true))
                assert(host.set_field_mode_effect_flag_for_setup(
                    effect.ILENE_INFORMATION_HEARD, true))
                assert(host.set_pending_world_effect_for_setup(true))
                assert(host.reenter_scene_for_setup(
                    E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR))
                return "seeded b18 story/gift and Ilene contact prerequisites, then staged Scene 20"
            end,
        },
        {
            id = "complete_ilene_activity_b19",
            milestone = 19,
            expected = {
                scene = E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR,
                facts = { [story.ILENE_MET] = true },
                flags = {
                    story = { set = { story.ILENE_MET } },
                    mode = { clear = { mode.BIT_31 } },
                    effect = {
                        set = { effect.ILENE_ACTIVITY_FOLLOWUP },
                        clear = { effect.ILENE_INFORMATION_HEARD },
                    },
                },
                karma_delta = { 2, 2, 2 },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR,
                    kIleneWorldX, kIleneWorldY)
                assert(placed, placement_error or "Ilene placement failed")
                assert(host.advance(24), "Ilene contact updates failed")
                local contact = host.snapshot()
                assert(contact.dialogue_open,
                    "Ilene's Scene-20 contact dialogue did not open")
                assert(contact.panel_text:find(
                    "Ilene gracefully leaps into a cat-like fighting stance",
                    1, true), "Ilene's live contact text was not shown")
                assert(contact.story_facts[story.RED_BUFFALO_WOMAN] == true,
                    "b18 test setup state was lost")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Ilene's contact did not return to the field")
                return "Ilene's live Scene-20 contact set b19, awarded two Karma, and returned to the field"
            end,
        },
    },
}
