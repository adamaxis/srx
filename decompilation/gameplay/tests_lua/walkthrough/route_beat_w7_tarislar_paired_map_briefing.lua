local E = require("enums")

local kSceneOneSpawnX = 0x2E00 -- Scene-1 start point before record 67 [C].
local kSceneOneSpawnY = 0x0D00
local kInteractionFrames = 80
local kDialogueWaitFrames = 120
local kConversationInputLimit = 24

local story = E.StoryFact
local effect = E.FieldModeEffectFlagBit
local renraku_map = E.GroupItemId.MAP_OVERLAY
local vigore_map = E.GroupItemId.LANDMARK_MAP

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_paired_map_briefing",
            milestone = 26,
            expected = {
                facts = {
                    [story.FROSTY_MET] = true,
                    [story.GARDEN_APTS_ENTERED] = true,
                    [story.ITO_FIGHT_WON] = true,
                    [story.POST_ITO_BRIEFED] = true,
                },
                group_items = { set = { renraku_map, vigore_map } },
                flags = {
                    story = { set = {
                        story.FROSTY_MET,
                        story.GARDEN_APTS_ENTERED,
                        story.ITO_FIGHT_WON,
                        story.POST_ITO_BRIEFED,
                    } },
                },
            },
            run = function(host)
                -- ROM writes FFFC1E bits 30/21/16/28 at 0x21FD0,
                -- 0x5456E, 0x54286, and 0x21F58 before paired-map info.
                for _, bit in ipairs({ E.StoryFlagBit.FROSTY_MET,
                        E.StoryFlagBit.GARDEN_APTS_ENTERED,
                        E.StoryFlagBit.ITO_FIGHT_WON,
                        E.StoryFlagBit.POST_ITO_BRIEFED }) do
                    local seeded, seed_error =
                        host.set_story_fact_for_setup(bit, true)
                    assert(seeded, seed_error or
                        "ROM paired-map prerequisite setup failed")
                end
                -- These inventory grants satisfy the ROM map-item checks at 0x220CC.
                assert(host.grant_group_item(renraku_map),
                    "Renraku Map Overlay test grant failed")
                assert(host.grant_group_item(vigore_map),
                    "Vigore Landmark Map test grant failed")
                return "seeded prior quest facts and granted the two ROM map items"
            end,
        },
        {
            id = "complete_paired_map_briefing_w7",
            milestone = 26,
            expected = {
                scene = E.Scene.PUYALLUP_BARRENS,
                facts = {
                    [story.RENRAKU_MAP_SHOWN] = true,
                    [story.BOTH_MAPS_SHOWN] = true,
                },
                group_items = { set = { renraku_map, vigore_map } },
                flags = {
                    story = { set = {
                        story.RENRAKU_MAP_SHOWN,
                        story.BOTH_MAPS_SHOWN,
                    } },
                    effect = { set = { effect.TOMB_MAPS_EXPLAINED } },
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
                    "real UP did not reach the repeat service at record 67")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Harlequin repeat service did not open")
                assert(host.snapshot().panel_option_count == 1,
                    "ROM paired-map information choice was not offered")

                for _ = 1, kConversationInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "live Scene-1 dialogue completed the paired-map explanation"
                    end
                    tap(host, "A")
                end
                error("paired-map dialogue did not return to Scene 1")
            end,
        },
    },
}
