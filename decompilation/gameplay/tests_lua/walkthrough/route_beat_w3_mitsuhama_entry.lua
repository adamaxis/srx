local E = require("enums")

local kMitsuhamaRecord95X = 11520
local kMitsuhamaRecord95Y = 3072

return {
    steps = {
        {
            id = "w3_enter_mitsuhama_after_hours",
            milestone = 1,
            expected = {
                scene = E.Scene.RESOURCE_ACTIVITY_30,
                flags = {
                    mode = { set = { E.FieldModeFlagBit.BIT_12 } },
                },
            },
            run = function(host)
                -- Mortimer's Aragorn answer sets mode bit 12 at ROM 0x2323A.
                local lead_set, lead_error =
                    host.set_field_mode_flag_for_setup(
                        E.FieldModeFlagBit.BIT_12, true)
                assert(lead_set, lead_error or
                    "ROM Mortimer lead setup failed")
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT,
                    kMitsuhamaRecord95X, kMitsuhamaRecord95Y)
                assert(placed, placement_error or
                    "Mitsuhama exterior setup placement failed")
                local entered, movement_error = host.hold_until_modal(
                    "UP", 600, E.Scene.RESOURCE_ACTIVITY_30)
                assert(entered, movement_error or
                    "real UP did not enter the after-hours Mitsuhama site")
                assert(host.wait_for_scene(
                    E.Scene.RESOURCE_ACTIVITY_30, 180),
                    "Mitsuhama record 95 did not load activity 30")
                return "entered activity 30 through Scene-3 record 95 with real UP"
            end,
        },
    },
}
