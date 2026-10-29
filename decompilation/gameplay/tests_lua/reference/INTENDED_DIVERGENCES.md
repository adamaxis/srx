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

`equiv_units` skips the functions in its INTENDED table. `equiv_heartbeat` is informational
(printed, not gating) in run_lua_suites.sh.

## advance_player_animation (actor/animator.lua)
Runner idle/walk lists from data/actor_anim_lists.lua (ROM 0x62AC) now keep their phase as a
normalized fraction. The 0x6092 rate uses +0xCA = 0x28 (C002+4 via 0xBABC). Idle's one-frame
list still advances its phase while holding its visible frame. `animator_tests.lua` exhausts all
source phase values against the visible frame cadence and verifies that runner state exposes no
source accumulator.

## advance_dynamic_actor_walk_animation (actor/anim.lua)
Dynamic actor locomotion also stores normalized phase. The source rate is still selected from the
movement step, including the below-0x16 multiplier and Gargoyle's frame-2 rate at 0x6092. Focused
`animator_tests.lua` checks both sides of the movement-step threshold and the Gargoyle special case.
