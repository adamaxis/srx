local E = require("enums")

-- Ilene's actor starts at ROM route 0xA8E4 (x=0x0E80, y=0x0700). The ROM
-- pair check at 0xEB92..0xEC46 opens contact once the runner's centre is
-- within 0x40 of hers on both axes. Approach from the open south side, one
-- 0x40 window plus a 0x24 margin below her; RIGHT from her west side is
-- deflected north by Scene-20 terrain and never enters the window.
local kIleneWorldX = 3712 -- 0x0E80 [C: 0xA8E4]
local kIleneApproachX = kIleneWorldX
local kIleneApproachY = 1892 -- 0x0700 + 0x64
local kContactInputFrames = 60

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
                -- event 0, whose route preparation (service 19 at 0x543A2,
                -- arm at 0x543D4..0x543EC) sets the transient FFE182 marker.
                -- Seed that marker, then re-enter so the production scene
                -- entry runs 0xA800: FFE180 clear + FFE182 set selects
                -- Ilene's actor list 0xA8CC (0xA82C..0xA842).
                assert(host.set_pending_world_effect_for_setup(true))
                local entered, entry_error = host.reenter_scene_for_setup(
                    E.Scene.HOLLYWOOD_CORRECTIONAL_INTERIOR)
                assert(entered, entry_error or
                    "Scene-20 setup re-entry failed")
                local seeded = host.snapshot()
                assert(seeded.script_actor_count == 1,
                    "Scene-20 entry did not install Ilene's 0xA8CC actor list")
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
                    "UP", kContactInputFrames)
                assert(opened,
                    "real UP did not trigger Ilene's Scene-20 contact")
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
