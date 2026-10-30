# Intended divergences from actor_ai_bfa3908.lua

The pre-split monolith is a refactor-equivalence reference. These ROM fixes change behavior on
purpose, so the reference is wrong for them:

| Commit | Behavior | ROM |
|---|---|---|
| 17130f9c | encounter actors spawn live; the end-of-combat gate waits for the DEAD edge | 0xAB02 |
| 623300eb | a zero stun/physical meter enters 0xB330 on the service pass | 0xA912..0xAA54 |
| a5a74ea3 | the target marker tiers the target's meter (<3, 3-5, 6-8, >=9) | 0x5C7E |
| (item 3) | the delayed close-range Hell Hound path resolves 0x55B16 and requests the 0xB146 child on changed defender meters | 0x14C7C, 0x13FFE, 0xB11C, 0x55B16 |
| (item 1) | default-list actors spawn live; the C0B2 death list is never played at spawn | 0x4618, 0xB330 |
| (item 1) | Wendigo (kind 0xE) and special-death actors deactivate after their list | 0xAA2C, 0x1E514 |
| (item 2) | action delay uses signed ASR for every nonzero D0 other than 2, sharing the player path | 0xABFC, 0xD7EA [C] |
| (movement API) | directional collision outcomes and route fallback directions cross into Lua as named facts; the native adapter retains collision encodings and doubled direction slots | 0xB732, 0xB77E, 0xBBC8 [C] |

`equiv_units` skips the functions in its INTENDED table. `equiv_heartbeat` is informational
(printed, not gating) in run_lua_suites.sh.

## advance_player_animation (actor/animator.lua)
Runner idle/walk lists from data/actor_anim_lists.lua (ROM 0x62AC) now keep their phase as a
normalized fraction. A named host cadence profile expresses 0.15625 frames per update, based on
the observed 0x6092 rate. Idle's one-frame list still advances its phase while holding its visible
frame. `animator_tests.lua` exhausts all source phase values against the visible frame cadence and
verifies that runner state exposes no source accumulator.

## advance_dynamic_actor_walk_animation (actor/anim.lua)
Dynamic actor locomotion also stores normalized phase. Named host cadence profiles express runner
frames per update, ordinary actor frames per map unit across speed bands, and the Gargoyle's slower
frame-2 timing. `animator_tests.lua` checks the full movement-distance range against the observed
cadence while gameplay avoids the source fixed-point arithmetic.

## advance_projectile_animation (actor/anim.lua)
Weapon-effect state now stores elapsed updates per frame and a semantic completion flag. The
three-frame and eleven-frame schedules are explicit host data derived from the visible cadence of
the ROM's 0x5FDC/0x600E path; Lua no longer mirrors its signed 16-bit clock or terminal selector bit.
Asset selectors remain in the host presentation adapter, while `animator_tests.lua` and the embedded
runtime QA assert the complete frame schedules and semantic completion behavior.

## Movement collision and route boundary
`game.movement.probe` returns `clear`, `requires_recovery`, or `blocked`, along
with the named fallback blocking fact and terrain-clear fact. Recovery receives
a compass direction; route lookup receives a cardinal direction and returns
marker-presence facts plus named fallback directions. The native adapter applies
the B77E/B732 collision rules and keeps the source collision value for B7B8
recovery. Formation following uses its own `follow_route_step_clear` service
because ROM 0x14550/0x1455A accepts nonpositive collision results. The embedded
runtime QA exercises preserved and unpreserved negative results, ordinary and
scene/mode/transition fallback outcomes, and native direction translation.
