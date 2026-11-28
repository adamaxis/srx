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
-- FFDF40 values: 0 outside (Save/Load allowed by 0xEFC4/0x596C4), 1 for an
-- event-table building scene (0x1684C).
local kOutsideInteractionMode = 0
local kBuildingInteractionMode = 1
local kInputFrames = 600
local kSceneTransitionFrames = 180
local kScene11ActivityActorId = "field_activity_scene_11_script_actor_0"
local kScene11Lone11Profile = "lone11_k04_item03"

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
                -- ROM list 0x17050 has one tagged Scene-11 row at
                -- 0x17158. The world-activity table resolves it to this
                -- lone11 template, source order 0, pool slot 3 [C].
                local population = host.snapshot()
                local actor_summaries =
                    population.script_actor_summaries or ""
                assert(not population.world_effect_active and
                       not population.pending_world_effect,
                    "Scene-11 entry unexpectedly armed an additional " ..
                    "0xA800 FFE124 list")
                assert(population.script_actor_count == 1,
                    "Scene-11 list 0x17050 expected one script actor; got " ..
                    tostring(population.script_actor_count) .. ": " ..
                    actor_summaries)
                assert(population.default_actor_count == 1,
                    "Scene-11 ROM population expected one actor across the " ..
                    "full pool; got " ..
                    tostring(population.default_actor_count) .. ": " ..
                    actor_summaries)
                assert(population.dynamic_actor_count == 0,
                    "Scene-11 ROM entry has no temporary encounter actors; " ..
                    "got " .. tostring(population.dynamic_actor_count))
                for _, expected in ipairs({
                    "id=" .. kScene11ActivityActorId,
                    "profile=" .. kScene11Lone11Profile,
                    "source_order=0",
                    "record_slot=3",
                }) do
                    assert(actor_summaries:find(expected, 1, true) ~= nil,
                        "Scene-11 list 0x17050 population mismatch: " ..
                        expected .. " in " .. actor_summaries)
                end
                -- 0x167AE sets FFDF40 = 1 for event-table scenes below
                -- 0x15 (0x1684C); 0xEFC4 then refuses Save/Load here, so a
                -- save can never be made without the door return point.
                local mode = host.snapshot().field_interaction_mode
                assert(mode == kBuildingInteractionMode,
                    "Big Rhino entry left FFDF40=" .. tostring(mode) ..
                    "; Save/Load would be allowed inside the building")
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
                -- The door return at 0x2F4E clears FFDF40 again.
                local mode = host.snapshot().field_interaction_mode
                assert(mode == kOutsideInteractionMode,
                    "Penumbra return left FFDF40=" .. tostring(mode))
                return "used Scene-11 records 8/9 with physical DOWN input"
            end,
        },
    },
}
