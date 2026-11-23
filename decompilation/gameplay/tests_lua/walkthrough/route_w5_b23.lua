local E = require("enums")

local kSeattleHospitalEventCenterX = 26880 -- Scene-0 record 52 center from ROM bounds 0x0680..0x069F [C]
local kSeattleHospitalEventCenterY = 14080 -- Scene-0 record 52 center from ROM bounds 0x0360..0x037F [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function show_holopix(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE,
        kSeattleHospitalEventCenterX, kSeattleHospitalEventCenterY)
    assert(placed, placement_error or "Seattle General placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or
        "Scene 0 hospital event 0 did not open with real Up input")
    assert(host.wait_for_dialogue(120),
        "Seattle General live prompt did not open")
    assert(host.snapshot().panel_actor == "Ivanna Haversheen",
        "Seattle General event 0 opened a different speaker")

    for _ = 1, 30 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 2 then
            tap(host, "B") -- Choose Later after the Stark question.
        else
            tap(host, "A") -- Show the holopix and ask the single Stark question.
        end
    end
    error("Seattle General holopix dialogue did not return to the field")
end

local fact = E.StoryFact.HOLOPIX_SHOWN
local recognition = E.FieldModeFlagBit.SEATTLE_HOLOPIX_RECOGNITION

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "show_holopix_at_seattle_general_b23",
            milestone = 23,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = { [fact] = true },
                flags = {
                    story = { set = { fact } },
                    mode = { clear = { recognition } },
                },
            },
            run = function(host)
                show_holopix(host)
                return "Ivanna's live hospital conversation set b23"
            end,
        },
    },
}
