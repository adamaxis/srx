local E = require("enums")

local kSceneOneSpawnX = 0x2E00 -- Scene-1 start point before record 67 [C].
local kSceneOneSpawnY = 0x0D00
local kInteractionFrames = 80
local kDialogueWaitFrames = 120
local kConversationInputLimit = 24

local story = E.StoryFact

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_post_ito_briefing",
            milestone = 26,
            expected = {
                facts = {
                    [story.FROSTY_MET] = true,
                    [story.GARDEN_APTS_ENTERED] = true,
                    [story.ITO_FIGHT_WON] = true,
                },
                flags = {
                    story = { set = {
                        story.FROSTY_MET,
                        story.GARDEN_APTS_ENTERED,
                        story.ITO_FIGHT_WON,
                    } },
                },
            },
            run = function(host)
                -- ROM writes bits 30/21/16 at 0x21FD0, 0x5456E, and 0x54286.
                for _, bit in ipairs({ E.StoryFlagBit.FROSTY_MET,
                        E.StoryFlagBit.GARDEN_APTS_ENTERED,
                        E.StoryFlagBit.ITO_FIGHT_WON }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM post-Ito prerequisite setup failed")
                end
                return "seeded Frosty, first-entry, and Ito-victory prerequisites"
            end,
        },
        {
            id = "complete_post_ito_briefing_w7",
            milestone = 26,
            expected = {
                scene = E.Scene.PUYALLUP_BARRENS,
                facts = { [story.POST_ITO_BRIEFED] = true },
                flags = {
                    story = { set = { story.POST_ITO_BRIEFED } },
                    effect = { set = {
                        E.FieldModeEffectFlagBit.HARLEQUIN_NO_RENRAKU_MAP_INFORMATION,
                    } },
                },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PUYALLUP_BARRENS,
                    kSceneOneSpawnX, kSceneOneSpawnY)
                assert(placed, placement_error or
                    "Scene-1 Tarislar approach placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not reach repeat service at record 67")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Harlequin post-Ito service did not open")
                assert(host.snapshot().panel_option_count == 2,
                    "ROM post-Ito choice menu was not shown")

                for _ = 1, kConversationInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "live Harlequin repeat service recorded the post-Ito briefing"
                    end
                    tap(host, "A")
                end
                error("post-Ito briefing dialogue did not return to Scene 1")
            end,
        },
    },
}
