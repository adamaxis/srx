local E = require("enums")

-- Scene-54 event record 0 at ROM 0x1E98EC/0x1E9912 bounds x=0x80..0x9F,
-- y=0x20..0x3F; scene world coordinates use 16 raw units per trigger unit.
local kTombExitApproachX = 0x0900 -- x=0x90, inside record 0.
local kTombExitApproachY = 0x0480 -- y=0x48, just south of record 0.
local kExitInputFrames = 600
local kDialogueWaitFrames = 180

local function tap(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

return {
    steps = {
        {
            id = "prepare_rom_tomb_departure_state",
            milestone = 28,
            expected = { scene = E.Scene.TARISLAR_TOMB },
            run = function(host)
                -- Setup only: the real Scene-1 tomb departure is covered by
                -- shadowrun_beat_w7_tarislar_tomb_departure. Its Yes command
                -- 0xE4 (0x24954) sets FFF0CC, then service 21 writes
                -- FFE186 = 1 at 0x545A6 before 0x167AE selects Scene 54.
                local seeded, seed_error =
                    host.set_field_scene_runtime_state_for_setup(
                        E.FieldSceneRuntimeState.TOMB_DEPARTURE)
                assert(seeded, seed_error or
                    "FFE186 tomb-departure state setup failed")
                return "seeded the ROM FFE186 tomb-departure state"
            end,
        },
        {
            id = "enter_scene_55_antechamber_w8",
            milestone = 28,
            expected = { scene = E.Scene.TARISLAR_TOMB_ANTECHAMBER },
            run = function(host)
                local before = host.snapshot()
                assert(before.scene == E.Scene.TARISLAR_TOMB,
                    "W8 entry test did not start inside Scene 54")
                local placed, placement_error = host.place_within_scene(
                    E.Scene.TARISLAR_TOMB,
                    kTombExitApproachX, kTombExitApproachY)
                assert(placed, placement_error or
                    "Scene-54 exit approach placement failed")
                local moved, movement_error = host.hold_until_modal(
                    "UP", kExitInputFrames)
                local after = host.snapshot()
                assert(after.scene == E.Scene.TARISLAR_TOMB_ANTECHAMBER,
                    movement_error or
                    "real UP through Scene-54 record 0 did not enter Scene 55")
                assert(moved, movement_error or
                    "Scene-54 exit did not start its entry continuation")
                assert(host.wait_for_dialogue(kDialogueWaitFrames),
                    "Scene-55 antechamber arrival page did not open")
                local arrival = host.snapshot()
                assert(arrival.panel_option_count == 1,
                    "ROM Scene-55 arrival page did not offer MORE")
                tap(host, "A")
                return "real Scene-54 UP reached Scene 55 and dismissed the arrival page"
            end,
        },
    },
}
