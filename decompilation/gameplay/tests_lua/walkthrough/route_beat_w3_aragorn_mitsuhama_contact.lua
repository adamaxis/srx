local E = require("enums")
local mode = E.FieldModeFlagBit

local kAragornContactX = 7424
local kAragornContactY = 16640

local function wait_panel(host, text)
    for _ = 1, 900 do
        if host.snapshot().panel_text:find(text, 1, true) then return end
        local ok, problem = host.advance(1)
        assert(ok, problem or "Aragorn contact dialogue advance failed")
    end
    error("Aragorn contact panel did not contain " .. string.format("%q", text))
end

local function advance(host, frames)
    local ok, problem = host.advance(frames)
    assert(ok, problem or "Aragorn contact frame advance failed")
end

return {
    steps = {
        {
            id = "w3_aragorn_mitsuhama_contact",
            milestone = 4,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_34,
                panel_text_contains = "get you outta' here",
                facts = { [E.StoryFact.ARAGORN_FOUND] = true },
                flags = {
                    story = { set = { E.StoryFact.OWLFEATHER_ASKED,
                        E.StoryFact.ARAGORN_FOUND } },
                    mode = {
                        set = { mode.BIT_13 },
                        clear = { mode.BIT_10, mode.BIT_12 },
                    },
                },
            },
            run = function(host)
                -- ROM 0x53C50 sets FFFC1E bit 2; the Scene-34 contact at
                -- 0x53C96 sets bit 3 after the actor is reached.
                for _, prerequisite in ipairs({
                    { E.StoryFlagBit.OWLFEATHER_ASKED, true },
                    { E.StoryFlagBit.ARAGORN_FOUND, false },
                }) do
                    local seeded, seed_error = host.set_story_fact_for_setup(
                        prerequisite[1], prerequisite[2])
                    assert(seeded, seed_error or
                        "ROM Aragorn story prerequisite setup failed")
                end
                -- Owlfeather command 0x3C at 0x21ED0 and Mortimer's answer at
                -- 0x2323A set search bits 10 and 12; 0x53C90 consumes them.
                for _, bit in ipairs({ mode.BIT_10, mode.BIT_12 }) do
                    local seeded, seed_error =
                        host.set_field_mode_flag_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM Aragorn mode prerequisite setup failed")
                end
                -- ROM 0x53C3C arms FFE180 for the later Scene-34 contact;
                -- 0xED7E consumes it when the live actor is contacted.
                assert(host.set_world_effect_active_for_setup(true,
                    E.FieldWorldEffectIndex.ARAGORN_CONTACT),
                    "ROM Aragorn contact effect setup failed")
                local entered, entry_error = host.reenter_scene_for_setup(
                    E.Scene.RESOURCE_ACTIVITY_34)
                assert(entered, entry_error or
                    "Scene-34 contact actor setup re-entry failed")
                advance(host, 30)
                assert(host.snapshot().script_actor_count >= 1,
                    "ROM Scene-34 story-contact setup installed no actors")

                local placed, placement_error = host.place_within_scene(
                    E.Scene.RESOURCE_ACTIVITY_34,
                    kAragornContactX, kAragornContactY)
                assert(placed, placement_error or
                    "Aragorn contact actor placement failed")
                local opened, dialogue_error = host.wait_for_dialogue(120)
                assert(opened, dialogue_error or
                    "Scene-34 contact actor did not open its dialogue")
                wait_panel(host, "Walking into the room")
                local pressed, press_error = host.press("A")
                assert(pressed, press_error or
                    "pad A did not advance Aragorn's contact dialogue")
                wait_panel(host, "get you outta' here")
                return "real pad A advanced Aragorn's live contact dialogue"
            end,
        },
    },
}
