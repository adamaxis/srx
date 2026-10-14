# ROM run-contract success paths

Status: **partial**. The ROM success rules are traced for all three
context-zero contract kinds. The Qt executable only connects package delivery;
it cannot yet produce a real ghoul defeat or spawn the bodyguard client.

## Offer records and shared payout

ROM `0x1E69C` selects one of eight six-byte records from `0xF3EBC` for the
current contract context. Context zero selects two ghoul-bounty rows, three
bodyguard rows, and three package-delivery rows. Each row carries a runner
progress delta of one. The negotiated payment is retained in `FFFBF4`.

The common payout at `0x1F00A` adds the negotiated payment to `FFFBFE` funds
and adds `FFFBF3` to byte `+0x8D` in all three runner records. When the run type
is zero, it multiplies the per-ghoul amount by the ghoul count in `FFFBF0`.
It then resets the active run state through `0x1E68C`.

## Ghoul bounty

- In ROM `0x559BC`, a defeated actor counts only while `FFFBED == 0` and its
  record byte at `+0x9B` is `0x42`. The increment occurs after the actor's
  damage value reaches zero. `FFFBF0` is capped at 20.
- The run handler at `0x1F1FC` sets bit 7 of `FFFC1C` only when `FFFBF0` is
  nonzero. This marks the bounty as ready to settle; this handler does not
  award funds.
- The area service at `0x20A12` maps field scene `0x0D` to context zero. It
  settles only when bit 7 is set and the active context `FFFBEC` equals the
  current context `FFF0AE`. It then calls `0x1F00A` and selects the result
  descriptor from the current context's row in `0x12CC50`. Context zero uses
  descriptor `0x105934` (`0x12CC50 + 8`).
- The existing map/contact audit identifies scene `0x0D` as Jump House and
  Gunderson. The ROM therefore defines bounty success as at least one eligible
  ghoul defeated, followed by reaching the matching Johnson context. This
  return uses a context match, not the package route's destination tuple.

## Bodyguard escort

- At the selected origin tuple, handler `0x1EF2C` sets bit 0 of `FFFC1C`,
  presents the origin page, and spawns the client through `0x1E4AC` using the
  actor list at `0x1D657C`.
- After pickup, a match to the selected destination tuple presents the
  destination page, calls `0x1DED8` to clear high effect slots, and proceeds
  to the shared payout.
- The traced destination block does not inspect the client record or test an
  alive flag. The separate actor lifecycle may affect other state; trace that
  before adding an alive requirement to Lua.

## Package delivery

- The selected origin tuple arms delivery. A later match to the selected
  destination tuple completes it. Context-zero setup retries the destination
  selection until it differs from the origin.
- The handler changes the alternate result and package-active flag, then pays
  once through `0x1F00A`. It requests no run-event sound, actor spawn, or
  animation.

## Current Lua and Qt connection

`run_package_delivery.lua` handles the origin and destination tuples, and the
field-exit dispatcher applies its typed result, funds, and runner progress.
`run_ghoul_bounty.lua` has no actor-defeat input or matching-context
settlement. `run_bodyguard_escort.lua` has no field-event resolver or connected
client spawn. The weapon-projectile path in `FieldSceneRuntime` currently
stops at `hit_resolution_pending`; it does not apply target damage or publish
an actor-defeat event. These are implementation gaps, not unknown ROM success
conditions.

## Visual evidence checklist

1. **Does it look correct?** Unknown for the result page at `0x105934`.
2. **Does visible state match behavior?** ROM conditions are traced; the live
   Qt run flow is only connected for package delivery.
3. **Are the visuals correct?** No run-result full-screen capture has been
   reviewed.
4. **Should it animate?** The package pickup/delivery branches request no
   animation. Ghoul and bodyguard completion visuals need full source/runtime
   review before calling them settled.
5. **Is there sound?** The package handler has no sound request. The other
   contract presentation and actor paths need separate audio tracing.
6. **Are adjacent screen elements clean?** No result transition has been
   captured, so replacement/overdraw is unverified.
7. **Do multiple frames support it?** Longplay frames cover the package offer,
   but not the full pickup-to-payout route or ghoul return.
8. **Are new controls drawn over old content?** No new controls were added in
   this source trace; no completion page is implemented.
9. **Was the affected time window reviewed continuously?** No. The result
   descriptor and complete return sequence still need full-screen capture.

A new gameplay video is not needed to recover these state predicates from the
ROM. It would help verify the completion page's appearance, animation/sound
timing, and the full visible return sequence.
