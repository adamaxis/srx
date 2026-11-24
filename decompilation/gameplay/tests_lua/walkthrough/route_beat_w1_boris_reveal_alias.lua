local E = require("enums")

local kGreenhouseRecord87X = 21248
local kGreenhouseRecord87Y = 1792
local kInformationFeeNuyen = 500

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        local state = host.snapshot()
        if state.panel_text:find(text, 1, true) then return end
        local ok, problem = host.advance(1)
        assert(ok, problem or "Boris dialogue advance failed")
    end
    error("Boris panel did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w1_boris_reveal_michael_alias",
            milestone = 1,
            expected = {
                scene = E.Scene.STOKER_MOTEL,
                panel_actor = "Boris",
                panel_text_contains = "using the name \"Owlfeather\"",
                nuyen_delta = 0,
                karma_delta = { 1, 1, 1 },
                facts = {
                    [E.StoryFact.MOTEL_BILL_PAID] = true,
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                    [E.StoryFact.MICHAEL_ALIAS_LEARNED] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.MOTEL_BILL_PAID,
                            E.StoryFact.TARISLAR_ASKED_SHALE,
                            E.StoryFact.MICHAEL_ALIAS_LEARNED,
                        },
                    },
                    mode = {
                        set = {
                            E.FieldModeFlagBit.BIT_5,
                            E.FieldModeFlagBit.BIT_8,
                            E.FieldModeFlagBit.BIT_20,
                        },
                        clear = {
                            E.FieldModeFlagBit.BIT_6,
                            E.FieldModeFlagBit.BIT_7,
                        },
                    },
                    dialogue = {
                        set = {
                            E.FieldDialogueRegisterFlagBit.BIT_1,
                            E.FieldDialogueRegisterFlagBit.BIT_2,
                        },
                    },
                },
            },
            run = function(host)
                local paid_seeded, paid_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.MOTEL_BILL_PAID, true)
                assert(paid_seeded, paid_error or
                    "ROM motel-payment setup failed")
                local shale_seeded, shale_error =
                    host.set_story_fact_for_setup(
                        E.StoryFlagBit.TARISLAR_ASKED_SHALE, true)
                assert(shale_seeded, shale_error or
                    "ROM Shale-question setup failed")
                -- The Inn's ROM command 0x14 at 0x21862..0x21886 sets these
                -- FFFC30 bits before the later Boris information contact.
                for _, bit in ipairs({
                    E.FieldModeFlagBit.BIT_5,
                    E.FieldModeFlagBit.BIT_6,
                    E.FieldModeFlagBit.BIT_8,
                }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM Shale-acceptance mode setup failed")
                end
                local reentered, reenter_error =
                    host.reenter_scene_for_setup(E.Scene.STOKER_MOTEL)
                assert(reentered, reenter_error or
                    "Redmond greenhouse setup reload failed")
                assert(host.grant_nuyen(kInformationFeeNuyen)) -- Setup grant: cover Boris's ROM 500-nuyen information fee.

                local placed, placement_error = host.place_within_scene(
                    E.Scene.STOKER_MOTEL,
                    kGreenhouseRecord87X, kGreenhouseRecord87Y)
                assert(placed, placement_error or
                    "Boris greenhouse setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened,
                    "Boris Scene-2 service did not open for record 87/event 6")
                wait_panel(host, "only illumination in the greenhouse")
                tap(host, "A")
                tap(host, "B")
                wait_panel(host, "I charge a flat fee of 500$")
                tap(host, "A")
                wait_panel(host, "Gives one a warm feeling all over")
                tap(host, "A")
                wait_panel(host, "using the name \"Owlfeather\"")
                return "paid for Boris's information and learned Michael's alias"
            end,
        },
    },
}
