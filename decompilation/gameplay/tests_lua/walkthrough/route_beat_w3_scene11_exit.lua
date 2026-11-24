local E = require("enums")

-- Scene-11 return records 8/9 (0x1D5AF8/0x1D5B02, code 0x0080) cover
-- x 512..1535, y 3584..4095 at the room's bottom-left opening; walls (code
-- 0x007F) close the rest of the south edge. Approach from inside the room
-- with DOWN.
local kExitApproachX = 1024
local kExitApproachY = 3300
-- Scene-3 record 98 at 0x18ABAA, event-table row 5, enters Scene 11 [C].
local kBigRhinoExteriorRecord98X = 9984
local kBigRhinoExteriorRecord98Y = 9472
local kInputFrames = 600
local kSceneTransitionFrames = 180

local function press(host, button)
    local ok, problem = host.press(button)
    assert(ok, problem or ("pad input failed: " .. button))
end

local function settle_bootstrap(host)
    for _ = 1, 600 do
        if not host.snapshot().opening_auto_walk then break end
        local advanced, problem = host.advance(1)
        assert(advanced, problem or "New Game opening setup advance failed")
    end
    assert(not host.snapshot().opening_auto_walk,
        "New Game opening walk did not settle before setup")
    if host.snapshot().dialogue_open then press(host, "A") end
    if host.snapshot().dialogue_open then press(host, "B") end
    if host.snapshot().dialogue_open then press(host, "C") end
    assert(not host.snapshot().dialogue_open,
        "bootstrap dialogue remained open before Scene 11 exit")
end

return {
    steps = {
        {
            -- Setup by real input: the building exit returns through the
            -- door route saved on entry, which a direct Scene-11 boot lacks.
            id = "w3_enter_big_rhino_for_exit",
            milestone = 3,
            expected = { scene = E.Scene.BIG_RHINO },
            run = function(host)
                settle_bootstrap(host)
                local placed, placement_error = host.place_within_scene(
                    E.Scene.PENUMBRA_DISTRICT,
                    kBigRhinoExteriorRecord98X, kBigRhinoExteriorRecord98Y)
                assert(placed, placement_error or
                    "Big Rhino exterior setup placement failed")
                local entered, movement_error = host.hold_until_modal(
                    "UP", kInputFrames, E.Scene.BIG_RHINO)
                assert(entered, movement_error or
                    "real UP did not select Scene-3 record 98")
                assert(host.wait_for_scene(E.Scene.BIG_RHINO,
                    kSceneTransitionFrames),
                    "Big Rhino event-table row 5 did not load Scene 11")
                return "entered the Big Rhino through Scene-3 record 98"
            end,
        },
        {
            id = "w3_scene11_physical_exit",
            milestone = 3,
            expected = {
                scene = E.Scene.PENUMBRA_DISTRICT,
                flags = { mode = { set = { E.FieldModeFlagBit.BIT_12 } } },
            },
            run = function(host)
                -- After the Aragorn question, ROM 0x2323A sets field-mode bit
                -- 12. Seed that post-dialogue condition for this isolated beat.
                local setup_ok, setup_error =
                    host.set_field_mode_flag_for_setup(
                        E.FieldModeFlagBit.BIT_12, true)
                assert(setup_ok, setup_error or
                    "Mortimer post-question mode setup failed")

                -- Scene-11 return records 8/9 are at 0x1D5AF8/0x1D5B02.
                local placed, placement_error = host.place_within_scene(
                    E.Scene.BIG_RHINO, kExitApproachX, kExitApproachY)
                assert(placed, placement_error or
                    "Scene 11 exit placement failed")
                local moved, movement_error = host.hold_until_modal(
                    "DOWN", kInputFrames, E.Scene.PENUMBRA_DISTRICT)
                assert(moved, movement_error or
                    "Scene 11 return did not open or transition")
                assert(host.wait_for_scene(E.Scene.PENUMBRA_DISTRICT,
                    kSceneTransitionFrames),
                    "Scene 11 exit did not return to Penumbra")
                return "used Scene-11 records 8/9 with physical DOWN input"
            end,
        },
    },
}
