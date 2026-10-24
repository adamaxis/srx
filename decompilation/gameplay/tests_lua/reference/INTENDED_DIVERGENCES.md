# Intended divergences from actor_ai_bfa3908.lua

The pre-split monolith is a refactor-equivalence reference. These ROM fixes change behavior on
purpose, so the reference is wrong for them:

| Commit | Behavior | ROM |
|---|---|---|
| 17130f9c | encounter actors spawn live; the end-of-combat gate waits for the DEAD edge | 0xAB02 |
| 623300eb | a zero stun/physical meter enters 0xB330 on the service pass | 0xA912..0xAA54 |
| a5a74ea3 | the target marker tiers the target's meter (<3, 3-5, 6-8, >=9) | 0x5C7E |
| (item 1) | default-list actors spawn live; the C0B2 death list is never played at spawn | 0x4618, 0xB330 |
| (item 1) | Wendigo (kind 0xE) and special-death actors deactivate after their list | 0xAA2C, 0x1E514 |

`equiv_units` skips the functions in its INTENDED table. `equiv_heartbeat` is informational
(printed, not gating) in run_lua_suites.sh.
