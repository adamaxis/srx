# ROM run-contract success paths

Status: **partial**. All five ROM offer tables and source initializer variants
now produce Lua-owned offers, including the three Matrix difficulty variants.
The connected New Game offer uses context 0, and the shared Wanderer Johnson
interaction uses context 3. Contexts 1, 2, and 4 are data-backed but their
location-specific conversation routes are not wired; scene 18's context-1
speaker is not yet identified.
Field completion remains incomplete for ghoul, escort, gang, recovery,
recruitment, and Matrix return-to-Johnson paths; package delivery is the only
connected completion route.

## Offer records and shared payout

ROM `0x1E69C` selects one of eight six-byte records from `0xF3EBC` for the
current contract context. The five contexts selected by the Johnson handler at
`0x20990` are 0 (scene 13), 1 (scene 18), 2 (scene 11), 3 (scene 14), and 4
(scene 12). `gameplay/scripts/data/johnson_run_offers.lua` contains all 40
records as named initializer, progress, payment, and negotiation fields. The
negotiated payment is retained with the offer and used by the common payout at
`0x1F00A`.

The common payout at `0x1F00A` adds the negotiated payment to `FFFBFE` funds
and adds `FFFBF3` to byte `+0x8D` in all three runner records. When the run type
is zero, it multiplies the per-ghoul amount by the ghoul count in `FFFBF0`.
It then resets the active run state through `0x1E68C`.

The source initializer table at `0xF3DC6` maps to canonical run handlers in
`0xF3E56`. Initializers 6/7/8 share canonical type 6 but select simple,
moderate, and expert Matrix bands. Initializers 9/10 are package-recovery route
variants; 11/12 are employee-recruitment route variants. The Lua offers keep
both the source initializer and canonical handler ID.

## Offer generation and presentation

- The registry chooses a source row from the selected context and dispatches
  to dedicated Lua modules for canonical run types 0 through 6. New Game still
  uses context 0.
- New Game selects context 0, and the connected Wanderer interaction selects
  context 3. The other contexts remain available in the typed source table,
  but their location-specific Johnson branches are not yet connected.
- Gang targets and descriptions use `0xF3E7A` and `0xF36FA`. Recovery and
  recruitment use the four-site, two-site, or fixed Renraku target records in
  `0xF3E6E`/`0xF3E76`; client and presentation rolls remain in Lua state.
- Matrix generation follows `0x1ED2C`: objective index, one of its eligible
  systems, retry until that system matches the initializer's difficulty band,
  then file size 10/20/30. Named objective text and system names use the
  decoded `0xF25AE`/`0xC19C6` data and related ROM tables. The common
  initializer clears run flags; target system, objective class/value, size,
  and typed completion flags live in the accepted Lua offer and run save.
- Offer pages use the shared Lua dialogue and Yes/No prompt. No full-screen
  offer capture has been reviewed, so page layout, clean replacement, and
  exact visual correspondence remain unverified.

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
The ghoul, escort, gang, package-recovery, employee-recruitment, and Matrix
modules now create and present source-backed offers, but their field results
intentionally do not claim completion. Ghoul bounty still needs actor-defeat
input and matching-context settlement; escort needs its source client spawn
and actor state; gang needs live encounter results; recovery/recruitment need
item/contact callbacks; Matrix needs a field return-settlement path. The
weapon-projectile path in `FieldSceneRuntime` still stops at
`hit_resolution_pending`, so it does not publish actor-defeat events.

## Visual evidence checklist

1. **Does it look correct?** Unknown for the result page at `0x105934`.
2. **Does visible state match behavior?** The Lua offer retains its ROM row,
   context, initializer, and canonical run type. Only package-delivery field
   completion is connected end to end.
3. **Are the visuals correct?** No run-result full-screen capture has been
   reviewed.
4. **Should it animate?** The package pickup/delivery branches request no
   animation. Ghoul and bodyguard completion visuals need full source/runtime
   review before calling them settled.
5. **Is there sound?** The package handler has no sound request. The other
   contract presentation and actor paths need separate audio tracing.
6. **Are adjacent screen elements clean?** The added text uses shared dialogue
   and prompt presenters, but offer captures have not checked replacement or
   overdraw across each new page.
7. **Do multiple frames support it?** Longplay frames cover the package offer,
   but not every offer variant or its full field route.
8. **Are new controls drawn over old content?** The offers use the existing
   shared Yes/No prompt; each new offer transition still needs a full-screen
   replacement check.
9. **Was the affected time window reviewed continuously?** No. The result
   descriptor and complete return sequence still need full-screen capture.

A new gameplay video is not needed to recover these state predicates from the
ROM. It would help verify the completion page's appearance, animation/sound
timing, and the full visible return sequence.
