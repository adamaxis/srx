local E = require("enums")

local kSceneOneSpawnX = 0x2E00 -- Scene-1 start point before record 67.
local kSceneOneSpawnY = 0x0D00
local kInteractionFrames = 80
local kDialogueWaitFrames = 120

local story = E.StoryFact

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_tomb_departure",
            milestone = 2,
            expected = {
                facts = {
                    [story.FROSTY_MET] = true,
                    [story.GARDEN_APTS_ENTERED] = true,
                    [story.ITO_FIGHT_WON] = true,
                    [story.POST_ITO_BRIEFED] = true,
                    [story.RENRAKU_MAP_SHOWN] = true,
                    [story.BOTH_MAPS_SHOWN] = true,
                },
                flags = {
                    story = { set = {
                        story.FROSTY_MET,
                        story.GARDEN_APTS_ENTERED,
                        story.ITO_FIGHT_WON,
                        story.POST_ITO_BRIEFED,
                        story.RENRAKU_MAP_SHOWN,
                        story.BOTH_MAPS_SHOWN,
                    } },
                },
            },
            run = function(host)
                -- ROM writes bits 30/21/16/28/26/27 at 0x21FD0, 0x5456E,
                -- 0x54286, 0x21F58, 0x2203E, and 0x2205C respectively.
                for _, bit in ipairs({ E.StoryFlagBit.FROSTY_MET,
                        E.StoryFlagBit.GARDEN_APTS_ENTERED,
                        E.StoryFlagBit.ITO_FIGHT_WON,
                        E.StoryFlagBit.POST_ITO_BRIEFED,
                        E.StoryFlagBit.RENRAKU_MAP_SHOWN,
                        E.StoryFlagBit.BOTH_MAPS_SHOWN }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM tomb-departure prerequisite setup failed")
                end
                return "seeded the ROM-gated Tarislar tomb prerequisites"
            end,
        },
        {
            id = "depart_tarislar_for_the_tomb_w7",
            milestone = 2,
            expected = { scene = E.Scene.TARISLAR_TOMB },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PUYALLUP_BARRENS,
                    kSceneOneSpawnX, kSceneOneSpawnY)
                assert(placed, placement_error or
                    "Scene-1 Tarislar approach placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not reach Scene-1 record 67")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Tarislar tomb-departure offer did not open")
                assert(host.snapshot().panel_option_count == 2,
                    "ROM tomb-departure Yes/No offer was not shown")
                assert(host.snapshot().panel_text:lower():find("tomb", 1, true),
                    "ROM tomb-departure prompt text was not shown")

                tap(host, "A") -- Accept the ROM Yes option.
                for _ = 1, 3 do
                    local state = host.snapshot()
                    assert(state.dialogue_open and state.panel_option_count == 1,
                        "ROM tomb travel page did not offer MORE")
                    tap(host, "A")
                end
                assert(host.wait_for_scene(E.Scene.TARISLAR_TOMB, 180),
                    "completed ROM tomb travel pages did not load Scene 54")
                return "live Yes and MORE inputs entered the Tarislar tomb scene"
            end,
        },
    },
}
