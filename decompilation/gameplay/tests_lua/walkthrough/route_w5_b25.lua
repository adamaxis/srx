local E = require("enums")

local kSeattleHospitalEventCenterX = 26880 -- Scene-0 record 52 center from ROM bounds 0x0680..0x069F [C]
local kSeattleHospitalEventCenterY = 14080 -- Scene-0 record 52 center from ROM bounds 0x0360..0x037F [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function admit_stark(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE,
        kSeattleHospitalEventCenterX, kSeattleHospitalEventCenterY)
    assert(placed, placement_error or "Seattle General placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or
        "Scene 0 hospital event 0 did not reopen with real Up input")
    assert(host.wait_for_dialogue(120),
        "Seattle General Stark dialogue did not open")
    assert(host.snapshot().panel_actor == "Ivanna Haversheen",
        "Seattle General event 0 opened a different speaker")

    for _ = 1, 40 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 2 then
            tap(host, "B") -- Choose Later on the closing offer.
        else
            tap(host, "A") -- Advance the Stark-found dialogue and its single choices.
        end
    end
    error("Seattle General Stark dialogue did not return to the field")
end

local fact = E.StoryFact
local fact_bit = E.StoryFlagBit
local effect = E.FieldModeEffectFlagBit

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "setup_b24_prerequisite_for_b25_test",
            milestone = 25,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = { [fact.MEDICINE_LODGE_TURNED_AWAY] = true },
                flags = {
                    story = { set = { fact.MEDICINE_LODGE_TURNED_AWAY } },
                },
            },
            run = function(host)
                assert(host.set_story_fact_for_setup(
                    fact_bit.MEDICINE_LODGE_TURNED_AWAY, true)) -- Route setup: seed b24 as a prerequisite; b25 remains live-tested.
                return "prepared b24 prerequisite for the independent b25 hospital test"
            end,
        },
        {
            id = "admit_stark_at_seattle_general_b25",
            milestone = 25,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                facts = { [fact.HOSPITAL_VISITED] = true },
                flags = {
                    story = { set = { fact.HOSPITAL_VISITED } },
                    effect = {
                        set = { effect.STARK_HEART_FAILING_TIP },
                        clear = { effect.STARK_DYING_TIP },
                    },
                },
            },
            run = function(host)
                assert(host.snapshot().story_facts[fact.HOLOPIX_SHOWN] == true,
                    "b23 holopix checkpoint prerequisite is missing")
                admit_stark(host)
                return "the live hospital path recorded Stark admitted at b25"
            end,
        },
    },
}
