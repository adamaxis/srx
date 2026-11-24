local E = require("enums")
local mode = E.FieldModeFlagBit

local kOwlfeatherLodgeCenterX = 21760
local kOwlfeatherLodgeCenterY = 20224

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Owlfeather dialogue advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Owlfeather panel did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w3_owlfeather_asks_about_michael",
            milestone = 3,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                panel_actor = "David Owlfeather",
                panel_text_contains = "You pull his holopix from your pocket",
                karma_delta = { 1, 1, 1 },
                facts = {
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                    [E.StoryFact.OWLFEATHER_ASKED] = true,
                },
                flags = {
                    story = {
                        set = {
                            E.StoryFact.TARISLAR_ASKED_SHALE,
                            E.StoryFact.OWLFEATHER_ASKED,
                        },
                    },
                    mode = { clear = { mode.BIT_8, mode.BIT_9 } },
                },
            },
            run = function(host)
                -- Service 2 at 0x53B78 needs FFFC1E bit 1 set, bit 2 clear.
                for _, prerequisite in ipairs({
                    { E.StoryFlagBit.TARISLAR_ASKED_SHALE, true },
                    { E.StoryFlagBit.OWLFEATHER_ASKED, false },
                }) do
                    local seeded, seed_error = host.set_story_fact_for_setup(
                        prerequisite[1], prerequisite[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather dialogue prerequisite setup failed")
                end
                -- Record 193 at 0x14C108 clears FFFC30 mode bits 8/9.
                for _, bit in ipairs({ mode.BIT_8, mode.BIT_9 }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM Owlfeather mode setup failed")
                end
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE,
                    kOwlfeatherLodgeCenterX, kOwlfeatherLodgeCenterY)
                assert(placed, placement_error or
                    "Owlfeather lodge setup placement failed")
                local opened, movement_error =
                    host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open Owlfeather's service")
                wait_panel(host, "How can I help you this fine day?")
                tap(host, "C")
                wait_panel(host, "You pull his holopix from your pocket")
                return "selected Owlfeather's live brother-information topic"
            end,
        },
    },
}
