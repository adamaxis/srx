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
            id = "w3_owlfeather_aragorn_task_briefing",
            milestone = 3,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                karma_delta = { 1, 1, 1 },
                facts = {
                    [E.StoryFact.TARISLAR_ASKED_SHALE] = true,
                    [E.StoryFact.OWLFEATHER_ASKED] = true,
                },
                flags = {
                    story = { set = {
                        E.StoryFact.TARISLAR_ASKED_SHALE,
                        E.StoryFact.OWLFEATHER_ASKED,
                    } },
                    mode = {
                        set = { mode.BIT_10 },
                        clear = { mode.BIT_8, mode.BIT_9 },
                    },
                },
            },
            run = function(host)
                -- Service 2 at ROM 0x53B78 requires FFFC1E bit 1 set and bit 2 clear.
                for _, prerequisite in ipairs({
                    { E.StoryFlagBit.TARISLAR_ASKED_SHALE, true },
                    { E.StoryFlagBit.OWLFEATHER_ASKED, false },
                }) do
                    local seeded, seed_error = host.set_story_fact_for_setup(
                        prerequisite[1], prerequisite[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather service prerequisite setup failed")
                end
                -- ROM service 0x53B78 clears scratch mode bits 8/9; command 0x3C
                -- at 0x21ED0 sets bit 10, which this live branch must produce.
                for _, flag in ipairs({
                    { mode.BIT_8, true },
                    { mode.BIT_9, true },
                    { mode.BIT_10, false },
                }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(flag[1], flag[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather mode prerequisite setup failed")
                end

                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE,
                    kOwlfeatherLodgeCenterX, kOwlfeatherLodgeCenterY)
                assert(placed, placement_error or
                    "Owlfeather lodge setup placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open Owlfeather's ROM service")
                assert(host.wait_for_dialogue(120),
                    "Owlfeather service dialogue did not open")
                wait_panel(host, "How can I help you this fine day?")

                tap(host, "C")
                wait_panel(host, "You pull his holopix from your pocket")
                for _, text in ipairs({
                    "I am sorry to hear of another senseless death",
                    "I have not made much progress",
                    "After his death, Tabatha gave me many leads",
                    "If you require his help, I must first ask for yours",
                    "Ironically, my own brother, Aragorn, left our land",
                }) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "C")
                wait_panel(host, "Sure thing.")
                tap(host, "A")
                wait_panel(host, "Farewell, and may the spirits guide you.")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Owlfeather briefing did not return to the field")
                return "live brother dialogue set the Aragorn search marker"
            end,
        },
    },
}
