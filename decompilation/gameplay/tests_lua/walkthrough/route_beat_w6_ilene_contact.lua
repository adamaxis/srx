local E = require("enums")

local kIleneApproachX = 3412 -- Reverse the measured 12-frame RIGHT vector into the ROM point.
local kIleneApproachY = 1852
local kContactInputFrames = 12

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
            id = "prepare_rom_state_for_ilene_contact",
            milestone = 19,
            expected = {
                scene = E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR,
                facts = { [story.RED_BUFFALO_WOMAN] = true },
                group_items = { set = { gillian_gift } },
                flags = {
                    story = { set = { story.RED_BUFFALO_WOMAN } },
                    mode = { set = { mode.BIT_31 } },
                    effect = { set = { effect.ILENE_INFORMATION_HEARD } },
                },
            },
            run = function(host)
                -- b18 service 0x5435C sets FFFC1E bit 18.
                local red_buffalo, red_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.RED_BUFFALO_WOMAN, true)
                assert(red_buffalo, red_error or
                    "b18 story prerequisite setup failed")
                -- The b18 gift exchange at 0x54362..0x54370 grants this item.
                assert(host.grant_group_item(gillian_gift))
                -- Ilene activity 0x54402 clears mode bit 31.
                local mode_seeded, mode_error = host.set_field_mode_flag_for_setup(
                    mode.BIT_31, true)
                assert(mode_seeded, mode_error or
                    "Ilene mode prerequisite setup failed")
                -- Scene-20 handoff 0x543D4..0x543EC requires effect bit 0
                -- or Notebook tip 76 before it arms FFE182.
                local effect_seeded, effect_error =
                    host.set_field_mode_effect_flag_for_setup(
                        effect.ILENE_INFORMATION_HEARD, true)
                assert(effect_seeded, effect_error or
                    "Ilene handoff prerequisite setup failed")
                -- The test starts directly in Scene 20, bypassing Scene-2
                -- service 19 at 0x543A2. Re-enter this map in setup so the
                -- production Scene-20 entry handler arms the FFE182 handoff.
                local entered, entry_error = host.reenter_scene_for_setup(
                    E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR)
                assert(entered, entry_error or
                    "Scene-20 setup re-entry failed")
                return "seeded the ROM b18 and Ilene-contact prerequisites"
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
                    kIleneApproachX, kIleneApproachY)
                assert(placed, placement_error or "Ilene placement failed")
                local opened = host.hold_until_modal(
                    "RIGHT", kContactInputFrames)
                assert(opened,
                    "real RIGHT did not trigger Ilene's Scene-20 contact")
                local contact = host.snapshot()
                assert(contact.dialogue_open,
                    "Ilene's Scene-20 contact dialogue did not open")
                assert(contact.panel_text:find(
                    "Ilene gracefully leaps into a cat-like fighting stance",
                    1, true), "Ilene's live contact text was not shown")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Ilene's contact did not return to the field")
                return "Ilene's live Scene-20 contact set b19 and awarded two Karma"
            end,
        },
    },
}
