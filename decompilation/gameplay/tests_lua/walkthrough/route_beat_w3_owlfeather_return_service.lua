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
    assert(ok, problem or "Owlfeather return dialogue advance failed")
end

local function wait_panel(host, text)
    for _ = 1, 900 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        advance(host, 1)
    end
    error("Owlfeather return panel did not contain " .. string.format("%q", text))
end

return {
    steps = {
        {
            id = "w3_owlfeather_return_service",
            milestone = 5,
            expected = {
                scene = E.Scene.SALISH_SHIDHE,
                karma_delta = { 2, 2, 2 },
                facts = {
                    [E.StoryFact.ARAGORN_FOUND] = true,
                    [E.StoryFact.ARAGORN_FREED_TOLD] = true,
                },
                flags = {
                    story = { set = {
                        E.StoryFact.ARAGORN_FOUND,
                        E.StoryFact.ARAGORN_FREED_TOLD,
                    } },
                    mode = {
                        set = { mode.BIT_14 },
                        clear = {
                            mode.BIT_8, mode.BIT_9, mode.BIT_10, mode.BIT_13,
                        },
                    },
                },
            },
            run = function(host)
                -- Service 4 at ROM 0x53CEA/0x53D5A requires FFFC1E bit 3 set
                -- and bit 4 clear; command 0x19 at 0x53D4E sets bit 4.
                for _, prerequisite in ipairs({
                    { E.StoryFlagBit.ARAGORN_FOUND, true },
                    { E.StoryFlagBit.ARAGORN_FREED_TOLD, false },
                }) do
                    local seeded, seed_error = host.set_story_fact_for_setup(
                        prerequisite[1], prerequisite[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather return-service prerequisite setup failed")
                end
                -- Service 4 clears mode bits 8/9/10/13 and sets bit 14.
                for _, flag in ipairs({
                    { mode.BIT_8, true },
                    { mode.BIT_9, true },
                    { mode.BIT_10, true },
                    { mode.BIT_13, true },
                    { mode.BIT_14, false },
                }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(flag[1], flag[2])
                    assert(seeded, seed_error or
                        "ROM Owlfeather return-service mode setup failed")
                end
                local placed, placement_error = host.place_within_scene(
                    E.Scene.SALISH_SHIDHE,
                    kOwlfeatherLodgeCenterX, kOwlfeatherLodgeCenterY)
                assert(placed, placement_error or
                    "Owlfeather return-service placement failed")
                local opened, movement_error = host.hold_until_modal("UP", 600)
                assert(opened, movement_error or
                    "real UP did not open Owlfeather's return service")
                assert(host.wait_for_dialogue(120),
                    "Owlfeather return-service dialogue did not open")
                wait_panel(host, "Still no luck, eh?")
                tap(host, "A")
                wait_panel(host, "The station doors swing wide")
                for _, text in ipairs({
                    "Owlfeather quickly clasps Aragorn",
                    "Brother, welcome back to our land",
                    "You must forgive him",
                    "Go to the Medicine Lodge",
                }) do
                    tap(host, "A")
                    wait_panel(host, text)
                end
                tap(host, "C") -- Thanks exits the return-service topic menu.
                wait_panel(host, "Sure thing.")
                tap(host, "A")
                wait_panel(host, "Farewell, and may the spirits guide you.")
                tap(host, "A")
                assert(not host.snapshot().dialogue_open,
                    "Owlfeather reunion did not return to the field")
                return "live service and reunion dialogue set the story fact and Karma"
            end,
        },
    },
}
