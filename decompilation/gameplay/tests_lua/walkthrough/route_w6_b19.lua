local E = require("enums")

local kIleneWorldX = 3712 -- ROM actor route 0xA8E4 positions Ilene at x=0x0E80 [C]
local kIleneWorldY = 1792 -- ROM actor route 0xA8E4 positions Ilene at y=0x0700 [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local story = E.StoryFact
local mode = E.FieldModeFlagBit
local effect = E.FieldModeEffectFlagBit
local gillian_gift = E.GroupItemId.GIFT_LADY_GILLIAN

return {
    resume_from_fixture_step = 1,
    steps = {
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
                local before = host.snapshot()
                assert(before.scene == E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR,
                    "b19 requires a production-reached Scene-20 checkpoint")
                assert(before.story_facts[story.RED_BUFFALO_WOMAN] == true and
                       before.group_items[gillian_gift] == true,
                    "live b18 story state or Lady Gillian's gift is missing")
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
                    "b18 story state was lost during Ilene's contact")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Ilene's contact did not return to the field")
                return "Ilene's live Scene-20 contact set b19, awarded two Karma, and returned to the field"
            end,
        },
    },
}
