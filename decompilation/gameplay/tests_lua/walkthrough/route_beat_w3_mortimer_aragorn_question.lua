local E = require("enums")

local kMortimerBoothX = 4608
local kMortimerBoothY = 1536
-- Question builder 0x2317A offers Aragorn only when FFFC1E bit 2 is set
-- (0x231F2) and bit 3 is clear (0x231FA). W3 sets bit 2 when Owlfeather is
-- asked about Michael, before the Big Rhino visit.
local kOwlfeatherAskedStoryBit = 2

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Mortimer dialogue advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        local state = host.snapshot()
        if state.panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Mortimer panel did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w3_mortimer_asks_about_aragorn",
            milestone = 1,
            expected = {
                scene = E.Scene.BIG_RHINO,
                panel_actor = "Mortimer Reed",
                panel_text_contains = "fragged a solo run against Mitsuhama",
                facts = {
                    [E.StoryFact.MOTEL_BILL_PAID] = true,
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                    [E.StoryFact.MICHAEL_ALIAS_LEARNED] = true,
                    [E.StoryFact.OWLFEATHER_ASKED] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.MOTEL_BILL_PAID,
                            E.StoryFact.TARISLAR_ASKED_SHALE,
                            E.StoryFact.MICHAEL_ALIAS_LEARNED,
                            E.StoryFact.OWLFEATHER_ASKED,
                        },
                    },
                    mode = {
                        set = { E.FieldModeFlagBit.BIT_12 },
                        clear = { E.FieldModeFlagBit.BIT_11 },
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
                for _, prerequisite in ipairs({
                    E.StoryFlagBit.MOTEL_BILL_PAID,
                    E.StoryFlagBit.TARISLAR_ASKED_SHALE,
                    E.StoryFlagBit.MICHAEL_ALIAS_LEARNED,
                    kOwlfeatherAskedStoryBit,
                }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(prerequisite, true)
                    assert(seeded, seed_error or
                        "ROM W1 story prerequisite setup failed")
                end
                -- Boris command 0x2B at 0x21B82 establishes the known
                -- Mortimer greeting through dialogue-register bits 1 and 2.
                for _, bit in ipairs({
                    E.FieldDialogueRegisterFlagBit.BIT_1,
                    E.FieldDialogueRegisterFlagBit.BIT_2,
                }) do
                    local seeded, seed_error =
                        host.set_field_dialogue_flag_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM Boris dialogue prerequisite setup failed")
                end
                local placed, placement_error = host.place_within_scene(
                    E.Scene.BIG_RHINO, kMortimerBoothX, kMortimerBoothY)
                assert(placed, placement_error or
                    "Mortimer booth setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open Mortimer's booth")
                wait_panel(host, "You move into the booth")
                tap(host, "A")
                -- ROM 0x209BE arms FFF112 on every Scene 11 visit, so even a
                -- known contact gets root 0x105558, not repeat 0x105574.
                wait_panel(host, "Well, what have we here")
                tap(host, "A")
                tap(host, "A")
                wait_panel(host, "However, I do know a valuable contact")
                tap(host, "A")
                tap(host, "C")
                local choices = host.snapshot()
                assert(choices.panel_option_count == 2 and
                    choices.panel_options[1]:find("Mako", 1, true) and
                    choices.panel_options[2]:find("Aragorn", 1, true),
                    "Mortimer's source question choices were not Mako then Aragorn")
                tap(host, "B")
                wait_panel(host, "fragged a solo run against Mitsuhama")
                return "selected Aragorn from Mortimer's live question menu"
            end,
        },
    },
}
