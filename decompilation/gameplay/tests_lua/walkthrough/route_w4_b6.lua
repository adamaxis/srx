local E = require("enums")

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local scene = E.Scene.COUNCIL_ISLAND
local fact = E.StoryFact
local mode = E.FieldModeFlagBit
local auxiliary = E.FieldAuxiliaryFlagBit
local horn = E.GroupItemId.GARGOYLE_HORN
local pelt = E.GroupItemId.HELL_HOUND_PELT

return {
    resume_from_fixture_step = 1,
    steps = {
        {
            id = "grant_gargoyle_horn_for_b6",
            milestone = 6,
            expected = { scene = scene, group_items = { set = { horn } } },
            run = function(host)
                assert(host.grant_group_item(horn)) -- Route setup grant: provide the b6 turn-in Horn without combat grinding.
                return "granted the Horn for the live Spirit Eyes b6 service"
            end,
        },
        {
            id = "turn_in_gargoyle_horn_b6",
            milestone = 6,
            expected = {
                scene = scene,
                group_items = { clear = { horn } },
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
                local placed, placement_error = host.place_within_scene(
                    scene, 27904, 2032)
                assert(placed, placement_error or "Spirit Eyes placement failed")
                local opened, movement_error = host.hold_until_modal("DOWN", 600)
                assert(opened, movement_error or
                    "real DOWN did not reach the Hell Hound Pelt service")
                assert(host.wait_for_dialogue(120),
                    "Spirit Eyes Horn service did not open dialogue")
                for _ = 1, 20 do
                    if not host.snapshot().dialogue_open then return "Spirit Eyes completed the live b6 Horn service" end
                    tap(host, "A")
                end
                error("Spirit Eyes Horn service did not return to the field")
            end,
        },
        {
            id = "grant_hell_hound_pelt_for_b7",
            milestone = 7,
            gate = false,
            expected = { scene = scene, group_items = { set = { pelt } } },
            run = function(host)
                assert(host.grant_group_item(pelt)) -- Route setup grant: provide the b7 turn-in pelt without combat grinding.
                return "granted the pelt after the b6 gate is cleared"
            end,
        },
        {
            id = "turn_in_hell_hound_pelt_b7",
            milestone = 7,
            gate = false,
            expected = {
                scene = scene,
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
            run = function()
                error("b7 remains gated until the b6 physical service passes")
            end,
        },
    },
}
