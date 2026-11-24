local E = require("enums")

local kHospitalEventCenterX = 26880 -- Scene-0 record 52 bounds 0x0680..0x069F [C]
local kHospitalEventCenterY = 14080 -- Scene-0 record 52 bounds 0x0360..0x037F [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 40

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "seed_rom_holopix_prerequisite_for_stark_admission",
            milestone = 25,
            expected = {
                facts = {
                    [E.StoryFact.HOLOPIX_SHOWN] = true,
                    [E.StoryFact.MEDICINE_LODGE_TURNED_AWAY] = true,
                },
                flags = {
                    story = { set = {
                        E.StoryFact.HOLOPIX_SHOWN,
                        E.StoryFact.MEDICINE_LODGE_TURNED_AWAY,
                    } },
                },
            },
            run = function(host)
                -- Setup only: Seattle General reads FFFC1E bit 23 at
                -- 0x1F878; command 0x3E at 0x21EF8 sets it after holopix.
                local seeded, seed_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.HOLOPIX_SHOWN, true)
                assert(seeded, seed_error or
                    "ROM holopix prerequisite setup failed")
                -- The admitted branch at 0x1F8AE requires bit 24, set on the
                -- first Stark location visit by service 0x1FDEA.
                local found, found_error = host.set_story_fact_for_setup(
                    E.StoryFlagBit.MEDICINE_LODGE_TURNED_AWAY, true)
                assert(found, found_error or
                    "ROM Stark-found prerequisite setup failed")
                return "seeded the ROM-recorded holopix prerequisite"
            end,
        },
        {
            id = "admit_stark_at_seattle_general_b25",
            milestone = 25,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = { [E.StoryFact.HOSPITAL_VISITED] = true },
                flags = {
                    story = { set = { E.StoryFact.HOSPITAL_VISITED } },
                    effect = {
                        set = { E.FieldModeEffectFlagBit.STARK_HEART_FAILING_TIP },
                        clear = { E.FieldModeEffectFlagBit.STARK_DYING_TIP },
                    },
                },
            },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE,
                    kHospitalEventCenterX, kHospitalEventCenterY)
                assert(placed, placement_error or
                    "Seattle General event placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not open Seattle General")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Seattle General Stark dialogue did not open")
                assert(host.snapshot().panel_actor == "Ivanna Haversheen",
                    "Scene-0 record 52 opened a different speaker")

                for _ = 1, kConversationInputLimit do
                    local state = host.snapshot()
                    if not state.dialogue_open then
                        return "the live hospital path recorded Stark admitted at b25"
                    end
                    tap(host, state.panel_option_count == 2 and "B" or "A")
                end
                error("Seattle General b25 conversation did not finish")
            end,
        },
    },
}
