local E = require("enums")

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function place_and_enter(host)
    local placed, placement_error = host.place_within_scene(
        E.Scene.COUNCIL_ISLAND, 27904, 2032)
    assert(placed, placement_error or "Spirit Eyes placement failed")
    local opened, movement_error = host.hold_until_modal("DOWN", 600)
    assert(opened, movement_error or
        "real DOWN did not reach the Hell Hound Pelt service")
    assert(host.wait_for_dialogue(120),
        "Spirit Eyes pelt service did not open dialogue")
    for _ = 1, 20 do
        if not host.snapshot().dialogue_open then return end
        tap(host, "A")
    end
    error("Spirit Eyes pelt service did not return to the field")
end

local fact = E.StoryFact
local mode = E.FieldModeFlagBit
local auxiliary = E.FieldAuxiliaryFlagBit
local pelt = E.GroupItemId.HELL_HOUND_PELT

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "setup_b6_prerequisite_for_b7_test",
            milestone = 7,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                facts = { [fact.SHAMAN_ITEM4_GIVEN] = true },
                flags = {
                    story = { set = { fact.SHAMAN_ITEM4_GIVEN } },
                    mode = {
                        set = { mode.BIT_17 },
                        clear = { mode.BIT_16 },
                    },
                    auxiliary = {
                        set = { auxiliary.SHAMAN_RITUAL_STAGE_1 },
                        clear = { auxiliary.SHAMAN_RITUAL_STAGE_0 },
                    },
                },
            },
            run = function(host)
                assert(host.set_story_fact_for_setup(
                    E.StoryFlagBit.SHAMAN_ITEM4_GIVEN, true)) -- Route setup: b6 is a prerequisite only; the b7 service remains live-tested.
                assert(host.set_field_mode_flag_for_setup(mode.BIT_16, false))
                assert(host.set_field_mode_flag_for_setup(mode.BIT_17, true))
                assert(host.set_field_auxiliary_flag_for_setup(
                    auxiliary.SHAMAN_RITUAL_STAGE_0, false))
                assert(host.set_field_auxiliary_flag_for_setup(
                    auxiliary.SHAMAN_RITUAL_STAGE_1, true))
                return "prepared b6 predecessor state for the independent b7 service test"
            end,
        },
        {
            id = "grant_hell_hound_pelt_for_b7_service",
            milestone = 7,
            expected = { scene = E.Scene.COUNCIL_ISLAND,
                group_items = { set = { pelt } } },
            run = function(host)
                assert(host.grant_group_item(pelt)) -- Route setup grant: provide the quest pelt without combat grinding.
                return "granted the pelt for the live Spirit Eyes service"
            end,
        },
        {
            id = "turn_in_hell_hound_pelt_b7",
            milestone = 7,
            expected = {
                scene = E.Scene.COUNCIL_ISLAND,
                group_items = { clear = { pelt } },
                facts = { [fact.SHAMAN_PELT_GIVEN] = true },
                flags = {
                    story = { set = { fact.SHAMAN_PELT_GIVEN } },
                    mode = {
                        set = { mode.BIT_18 },
                        clear = { mode.BIT_17 },
                    },
                    auxiliary = {
                        set = { auxiliary.SHAMAN_RITUAL_STAGE_2 },
                        clear = { auxiliary.SHAMAN_RITUAL_STAGE_1 },
                    },
                },
            },
            run = function(host)
                place_and_enter(host)
                return "Spirit Eyes consumed the pelt and advanced the ritual to b7"
            end,
        },
    },
}
