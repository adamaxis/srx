local E = require("enums")

local kTomb = E.Scene.TARISLAR_TOMB
local kAntechamber = E.Scene.TARISLAR_TOMB_ANTECHAMBER
local kExitX = 0x0900
local kExitY = 0x0480
local kExitFrames = 600
local kOutroWaitFrames = 1800

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_tomb_departure_state_for_live_outro",
            milestone = 29,
            expected = { scene = kTomb },
            run = function(host)
                -- ROM service 21 writes FFE186=1 at 0x545A6 before scene 54;
                -- the existing W7 route verifies that service. Seed only that
                -- pre-transition state, not the ending result.
                local seeded, seed_error =
                    host.set_field_scene_runtime_state_for_setup(
                        E.FieldSceneRuntimeState.TOMB_DEPARTURE)
                assert(seeded, seed_error or
                    "ROM tomb departure state setup failed")
                return "seeded ROM FFE186=1 before the live Scene-54 exit"
            end,
        },
        {
            id = "enter_scene_55_through_real_exit_for_live_outro",
            milestone = 29,
            expected = { scene = kAntechamber },
            run = function(host)
                local placed, placement_error = host.place_within_scene(
                    kTomb, kExitX, kExitY)
                assert(placed, placement_error or
                    "could not place within Scene 54 exit bounds")
                local moved, movement_error = host.hold_until_modal(
                    "UP", kExitFrames, kAntechamber)
                assert(host.snapshot().scene == kAntechamber,
                    movement_error or
                    "real Scene-54 UP did not enter Scene 55")
                assert(moved, movement_error or
                    "Scene-54 exit did not start its live continuation")
                assert(host.wait_for_dialogue(180),
                    "Scene-55 arrival page did not open")
                local arrival = host.snapshot()
                assert(arrival.panel_option_count == 1,
                    "Scene-55 arrival page did not offer its ROM MORE choice")
                tap(host, "A")
                return "real Scene-54 UP reached Scene 55; dismissed arrival with pad A"
            end,
        },
        {
            id = "advance_live_scene_55_outro_to_warning_boundary",
            milestone = 29,
            expected = { scene = kAntechamber },
            run = function(host)
                local wait_ok, warning_opened = pcall(function()
                    return host.wait_for_dialogue(kOutroWaitFrames)
                end)
                if not wait_ok or not warning_opened then
                    local state = host.snapshot()
                    error("live Scene-55 outro did not open the warning; " ..
                        "FFE186=" .. tostring(state.field_scene_runtime_state) ..
                        "; FFF140=" ..
                            tostring(state.world_frame_transition_marker) ..
                        "; slot-3 meter=" ..
                            tostring(state.world_frame_slot_three_meter) ..
                        "; wait=" .. tostring(warning_opened))
                end
                local warning = host.snapshot()
                assert(warning.page_key ==
                    "tarislar.tomb.ending.page.licourtrix_warning",
                    "Scene-55 production path opened an unexpected dialogue")
                local confirmed, confirm_error = host.press("A", 1, 0)
                assert(confirmed, confirm_error or
                    "real A did not reach the ending presentation boundary")
                return "live Scene-55 outro opened the warning; real A confirmed it"
            end,
        },
    },
}
