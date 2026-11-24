local E = require("enums")

local kSeattleHospitalEventX = 26880 -- Scene-0 record 52 ROM bounds [C]
local kSeattleHospitalEventY = 14080 -- Scene-0 record 52 ROM bounds [C]
local kInteractionFrames = 600
local kDialogueWaitFrames = 120
local kConversationInputLimit = 30

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "w5_b23_show_holopix_to_ivanna",
            milestone = 23,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = { [E.StoryFact.HOLOPIX_SHOWN] = true },
                flags = {
                    story = { set = { E.StoryFact.HOLOPIX_SHOWN } },
                    mode = {
                        clear = {
                            E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION,
                        },
                    },
                },
            },
            run = function(host)
                -- ROM hospital handler 0x1F878 checks recognition bit 3;
                -- action 0x21EF8 clears it after showing the holopix.
                local ready, ready_error = host.set_field_mode_flag_for_setup(
                    E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION, true)
                assert(ready, ready_error or
                    "Seattle holopix prerequisite setup failed")

                local placed, placement_error = host.place_within_scene(
                    E.Scene.DOWNTOWN_SEATTLE,
                    kSeattleHospitalEventX, kSeattleHospitalEventY)
                assert(placed, placement_error or
                    "Seattle General event placement failed")
                local opened, movement_error = host.hold_until_modal(
                    "UP", kInteractionFrames)
                assert(opened, movement_error or
                    "real UP did not open Seattle General")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Seattle General dialogue did not open")
                assert(host.snapshot().panel_actor == "Ivanna Haversheen",
                    "Scene-0 record 52 opened a different speaker")

                for _ = 1, kConversationInputLimit do
                    local state = host.snapshot()
                    if not state.dialogue_open then
                        return "Ivanna's live holopix conversation recorded b23"
                    end
                    tap(host, state.panel_option_count == 2 and "B" or "A")
                end
                error("Seattle General b23 conversation did not finish")
            end,
        },
    },
}
