local E = require("enums")

local kSceneOneSpawnX = 0x2E00 -- Scene-1 QA start point before record 67.
local kSceneOneSpawnY = 0x0D00
local kInteractionFrames = 80
local kDialogueWaitFrames = 120
local kDialogueInputLimit = 32

local story = E.StoryFact
local effect = E.FieldModeEffectFlagBit

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_state_for_tarislar_first_entry",
            milestone = 1,
            expected = {
                facts = { [story.FROSTY_MET] = true },
                flags = {
                    story = { set = { story.FROSTY_MET } },
                    effect = { set = { effect.FROSTY_HARLEQUIN_EXPOSITION } },
                },
            },
            run = function(host)
                -- Frosty's command 0x44 at 0x21FD0 sets FFFC1E bit 30.
                local frosty, frosty_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.FROSTY_MET, true)
                assert(frosty, frosty_error or "Frosty story prerequisite setup failed")
                -- Command 0x49 at 0x220EA sets FFFC34 bit 5 before the visit.
                local effect_seeded, effect_error =
                    host.set_field_mode_effect_flag_for_setup(
                        effect.FROSTY_HARLEQUIN_EXPOSITION, true)
                assert(effect_seeded, effect_error or
                    "Harlequin exposition effect setup failed")
                return "seeded ROM Frosty prerequisites for first entry"
            end,
        },
        {
            id = "enter_tarislar_garden_apartments_w7",
            milestone = 1,
            expected = {
                scene = E.Scene.PUYALLUP_BARRENS,
                facts = { [story.GARDEN_APTS_ENTERED] = true },
                flags = {
                    story = { set = { story.GARDEN_APTS_ENTERED } },
                    effect = {
                        set = {
                            effect.GARDEN_APTS_FIRST_ENTRY,
                            effect.HARLEQUIN_NO_ITO_INFORMATION,
                        },
                        clear = { effect.FROSTY_HARLEQUIN_EXPOSITION },
                    },
                },
                karma_delta = { 2, 2, 2 },
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
                    "real UP did not reach Scene-1 record 67")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Tarislar first-entry dialogue did not open")
                assert(host.snapshot().panel_text:find(
                    "Tarlisar Garden Apartments", 1, true),
                    "ROM first-entry page text was not shown")

                for _ = 1, kDialogueInputLimit do
                    if not host.snapshot().dialogue_open then
                        return "real Scene-1 movement completed the Tarislar first-entry beat"
                    end
                    tap(host, "A")
                end
                error("Tarislar first-entry dialogue did not return to Scene 1")
            end,
        },
    },
}
