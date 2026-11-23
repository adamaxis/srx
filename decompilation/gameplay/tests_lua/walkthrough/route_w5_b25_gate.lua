local E = require("enums")

local kSeattleHospitalX = 26880 -- Scene-0 record 52 center, ROM bounds 0x0680..0x069F [C]
local kSeattleHospitalY = 14080 -- Scene-0 record 52 center, ROM bounds 0x0360..0x037F [C]
local fact = E.StoryFact
local effect = E.FieldModeEffectFlagBit

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function dismiss_default_hospital_offer(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE, kSeattleHospitalX, kSeattleHospitalY)
    assert(placed, placement_error or "Seattle General placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or
        "Scene-0 hospital event 0 did not open with real Up input")
    assert(host.wait_for_dialogue(120),
        "Seattle General b25 gate dialogue did not open")
    assert(host.snapshot().panel_actor == "Ivanna Haversheen",
        "Seattle General event 0 opened a different speaker")

    for _ = 1, 40 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 2 then
            tap(host, "B")
        else
            tap(host, "A")
        end
    end
    error("Seattle General b25 gate dialogue did not return to the field")
end

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "b25_stays_gated_without_scene7_exit",
            milestone = 25,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = {
                    [fact.MEDICINE_LODGE_TURNED_AWAY] = false,
                    [fact.HOSPITAL_VISITED] = false,
                },
                flags = {
                    story = { clear = {
                        fact.MEDICINE_LODGE_TURNED_AWAY,
                        fact.HOSPITAL_VISITED,
                    } },
                    effect = { clear = {
                        effect.STARK_DYING_TIP,
                        effect.STARK_HEART_FAILING_TIP,
                    } },
                },
            },
            run = function(host)
                local before = host.snapshot()
                assert(before.story_facts[fact.HOLOPIX_SHOWN] == true,
                    "b23 holopix checkpoint prerequisite is missing")
                assert(before.story_facts[fact.MEDICINE_LODGE_TURNED_AWAY] ~= true,
                    "b24 prerequisite unexpectedly came from the b23 fixture")
                dismiss_default_hospital_offer(host)
                return "hospital remained on its ordinary path; b25 is gated"
            end,
        },
    },
}
