# Player untargeted spell roll

## Source trace

- `[C]` `0xAD86` enters `0xAE6E` when the player action gate is negative. At
  `0xAE7A`, a zero `+0xA2` link branches directly to `0xAEB4`, which jumps to
  `0x1033E`. The line probe at `0x14D8E` is only run when the link is nonzero.
- `[C]` `0x1033E` skips the roll only when `+0x9C == 1`. Otherwise it computes
  the player's trial count, modifies difficulty, runs the threshold rolls at
  `0x0DBA`, writes the successes to `+0x9F`, dispatches `0x15830` on success,
  then enters the shared residual/fetish and meter path at `0x10660`.
- `[C]` With no link, `0x15830` checks action codes `0x07` and `0x0E` before
  reading `+0xA2`. Those two codes enter `0x49A2`. Every other code reads the
  zero link and returns at `0x1592E` before allocating a target effect, playing
  the spell-table sound, or running a target scan.
- `[C]` `0x49A2` clears an existing barrier, reserves a shared effect record,
  stores the heading and the duration `+0x9F * 0x0190`, writes five
  direction-relative floor cells, adds the field sprite, and plays event
  `0x3B` at `0x4AA0`. Code `0x07` writes floor code `0x7F` (Super Barrier);
  code `0x0E` leaves `0x7E` (Barrier).
- `[C]` Stink (`0x06`) and Confusion (`0x0D`) reach their area-target work only
  after `0x15830` has a nonzero link. With no link, neither the `0x1597E`
  target scan nor its group-result writes run. Heal Wounds (`0x0C`) can select
  a wounded runner in the NPC `0x13FFE` route. Before the player action,
  `0xACAE` may call `0x1DC4C` on input bit 12; for action codes `0x04`, `0x0B`,
  and `0x0C`, `0x1DC4C` follows its record-selection path at `0x1DD2E`. That
  helper uses `A2`, not `A1`. If `+0xA2` remains zero at `0xAE6E`, the spell
  resolver has no self-target or automatic heal branch. Invisibility and
  Rockskin therefore also return before a target effect on this no-link path.
- `[C]` `0xAE6E` does not assign `A1` when `+0xA2` is zero. `0xA8EA` invokes
  the player's `0xACAE` action before its `0xAAA6` load of the previous record's
  `+0x3A` callback into `A1`; `0xAAB0` then executes that callback, which may
  itself change `A1`. The first record inherits `A1` from the outer caller.
  The no-link path in `0x1033E` reaches `0x55740` without assigning `A1`, and
  `0x1DC4C` uses `A2` only. `0x55740` reads `+0x9B` and `+0x11` through the
  unchanged `A1`. Thus the stale defender bytes
  depend on earlier runtime state and are not a stable player target.

## Native reading

`[H]` Keep the confirmed player-side trial count, roll, result publication,
residual/fetish drain, and meter updates. Apply the known attacker-side
cyberware modifier, but omit defender-only adjustments from the stale `A1`
bytes. Do not fabricate a self target. On a successful no-link roll, codes
`0x07` and `0x0E` spawn the directional field barrier and event `0x3B`; other
codes keep the action result and drain but have no target effect, projectile,
or spell-table audio. Their cast pose and delay still run through `0xAF7C` or
`0xAFAE`.

The Lua suite `player_spell_no_target_tests.lua` checks every action code from
`0x01` through `0x0E`, including the two barrier records, the Stink/Confusion
early return, caster drain, no fabricated target, and barrier-slot expiry.
