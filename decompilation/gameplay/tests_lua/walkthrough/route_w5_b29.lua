local E = require("enums")

local kSeattleHospitalEventCenterX = 26880 -- Scene-0 record 52 center from ROM bounds 0x0680..0x069F [C]
local kSeattleHospitalEventCenterY = 14080 -- Scene-0 record 52 center from ROM bounds 0x0360..0x037F [C]

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function turn_in_cyberheart(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.DOWNTOWN_SEATTLE,
        kSeattleHospitalEventCenterX, kSeattleHospitalEventCenterY)
    assert(placed, placement_error or "Seattle General placement failed")
    local opened, movement_error = host.hold_until_modal("UP", 600)
    assert(opened, movement_error or
        "Scene 0 hospital event 0 did not open with real Up input")
    assert(host.wait_for_dialogue(120),
        "Seattle General heart-turn-in dialogue did not open")
    assert(host.snapshot().panel_actor == "Ivanna Haversheen",
        "Seattle General event 0 opened a different speaker")

    for _ = 1, 40 do
        local snapshot = host.snapshot()
        if not snapshot.dialogue_open then return end
        if snapshot.panel_option_count == 2 then
            tap(host, "B") -- Choose Later on the closing offer.
        else
            tap(host, "A") -- Advance the live heart handoff pages.
        end
    end
    error("Seattle General heart-turn-in dialogue did not return to the field")
end

local fact = E.StoryFact.HEART_DELIVERED
local effect = E.FieldModeEffectFlagBit.CYBERHEART_CARRIED_TIP
local heart = E.GroupItemId.PROTOTYPE_CYBERHEART

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "grant_cyberheart_for_b29_turn_in",
            milestone = 29,
            expected = { scene = E.Scene.DOWNTOWN_SEATTLE,
                group_items = { set = { heart } } },
            run = function(host)
                assert(host.grant_group_item(heart)) -- Route setup grant: provide the quest cyberheart without combat grinding.
                return "granted the turn-in item for Seattle General"
            end,
        },
        {
            id = "deliver_cyberheart_to_seattle_general_b29",
            milestone = 29,
            expected = {
                scene = E.Scene.DOWNTOWN_SEATTLE,
                group_items = { clear = { heart } },
                facts = { [fact] = true },
                flags = {
                    story = { set = { fact } },
                    effect = { clear = { effect } },
                },
            },
            run = function(host)
                local snapshot = host.snapshot()
                assert(snapshot.story_facts[E.StoryFact.HOSPITAL_VISITED] == true and
                       snapshot.story_facts[E.StoryFact.HOLOPIX_SHOWN] == true,
                    "b25/b23 hospital prerequisites are missing")
                turn_in_cyberheart(host)
                return "Ivanna consumed the cyberheart and recorded b29"
            end,
        },
    },
}
