# ROM run-contract success paths

Status: **partial**. All five ROM offer tables and source initializer variants
now produce Lua-owned offers, including the three Matrix difficulty variants.
The location routes are wired for New Game/context 0, Julius/context 1,
Mortimer/context 2, Wanderer/context 3, and Caleb/context 4. Julius's gated
questions and Quinton contact are implemented; Mortimer's gated information
and $1,500 Alesandro contact are implemented. Caleb's information/contact
branch, context-specific run returns beyond the connected Ghoul Bounty path,
and non-Ghoul context-2 settlement remain open.
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

The visible `FFFBF4` amount is `base_payment + 5 * roll`. Raw `0x1E69C` reads
the first live runner's Negotiation byte at `FF018C`, sets
`bound = negotiation_scale * skill + 1`, samples below that bound, then adds
five per result. The host `runner.negotiation_skill` service uses the first
party record and `RunnerSkillIndex::negotiation` (skill index 11, live byte
`+0x8C`), matching the source field. There is no direct class/race term in
this formula; the thirteen ROM presets differ through their Negotiation
values, which range from 0 to 4. The Attributes/Skills source permits ranks
0-12 for Negotiation; only Sorcery has the class restriction in that grid.

Pure-Lua `run_offer_payment_profile_tests.lua` checks all 40 source offer rows,
all 13 valid skill ranks, and both ends of each bounded roll (1,040 results).
That includes the actual preset range. The maximum generated amount across
these rows is 6,450 nuyen at preset rank 4 (base 6,350, scale 5); at the
upgrade cap, it is 6,700 (base 6,100, scale 10). The common detail builder at
`0x1EE64` passes `FFFBF4` to the grouped decimal renderer `0xC216` at `$C322`.
The raw formatter emits the least significant digits first and inserts the
group separator after every three digits, so the audited maximum is presented
as `6,700`. The Lua test verifies the amount across all source rows and ranks,
but it does not execute the ROM glyph formatter or verify visible pixels. No
independent Genesis Current Run screenshot has been identified. The Ghidra
export `fn_0000C216.cpp` agrees with the ROM loop and is only a cross-check.
`shadowrun_ghidra_decimal_glyph` checks that the host glyph sequence and
bitmap-font text render 6,450 and 6,700 as grouped values; this is internal
conversion coverage only. Profile values are listed in the [Attributes/Skills
profile audit](attributes-skills-profile-permutation-audit-2026-10-23.md).

### Independent longplay payment anchor

Independent 320x224 longplay frame
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_03_jump_house_stoker_job_0118-0238/frame_003300.png`
shows the Jump House offer with base `40` and negotiated payment `55`. Raw
context-zero row index 4 at `0xF3EBC` is Package Delivery, progress 1, base 40,
negotiation scale 1. Raw `0x2148A` calls `0x1E69C`; that routine stores base 40
in `FFF0C0` and computes `FFFBF4` from the first live runner's `FF018C`
Negotiation skill. The offer renderer passes both values to grouped decimal
formatter `0xC216` at `0x21526`/`0x2153C`.

The later same-session status frame
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_06_combat_inventory_delivery_0510-0600/frame_002550.png`
identifies Joshua as Human Shaman. The ROM preset `Joshua Gator Shaman` is
Human/Shaman with Negotiation 3. That makes the bound 4, and the visible
difference of 15 implies roll 3: `40 + 5 * 3 = 55`. The Lua matrix now locks
this row/profile/roll anchor, and the Debug/Release decimal-glyph CTest checks
the displayed values 40 and 55.

This is independent visual agreement for the Johnson offer path and its shared
payment calculation. The skill and roll are derived by joining the separate
profile frame and ROM preset with the displayed amount. The Notebook Current
Run detail page itself remains unimplemented and has no independent screenshot.

The source initializer table at `0xF3DC6` maps to canonical run handlers in
`0xF3E56`. Initializers 6/7/8 share canonical type 6 but select simple,
moderate, and expert Matrix bands. Initializers 9/10 are package-recovery route
variants; 11/12 are employee-recruitment route variants. The Lua offers keep
both the source initializer and canonical handler ID.

## Offer generation and presentation

- The registry chooses a source row from the selected context and dispatches
  to dedicated Lua modules for canonical run types 0 through 6. New Game still
  uses context 0.
- ROM `0x2148A` runs the selected offer initializer; `0x214B0` dispatches the
  offer text formatter through `0xF3E1E`. For types 0-6 those formatters are
  the same type-specific helper entries used by the Notebook Current Run
  handlers, tying the offer text and Current Run text to the same live fields.
- Ghoul initializer `0x1E744` sets `FFFBEE=2` for context 0 and rolls the area
  below 4 for other contexts; Current Run helper `0x1E78C` renders the area
  selected by that field. Lua already retained that selection as
  `offer.area_id`, but `run_ghoul_bounty.present_offer` had always displayed
  area 2. It now uses the stored area with an invalid-area guard. The focused
  Debug/Release `shadowrun_ghidra_run_actor_defeat` test checks context 0's
  Redmond branch and all four Mortimer/context-2 area rolls (0-3), verifying
  each corresponding area label. This verifies offer text generation, not the
  unimplemented Notebook Current Run detail screen.
- New Game selects context 0; the scene-18, scene-11, scene-14, and scene-12
  Johnson profiles select contexts 1 through 4. Their work branches read the
  same typed offer tables; each location's dialogue and question gates stay in
  its Lua content profile.
- Gang targets and descriptions use `0xF3E7A` and `0xF36FA`. Recovery and
  recruitment use the four-site, two-site, or fixed Renraku target records in
  `0xF3E6E`/`0xF3E76`; client and presentation rolls remain in Lua state.
- The Employee Recruitment generated-offer text is audited across all seven
  source target tuples and all twelve client names. The Johnson offer screenshot
  exposed an extra period before the client separator (`Seattle., Jorge`): ROM
  `0x1E91A` changes the selected `0xF1E58` area's terminal period to a comma.
  The offer presenter now matches that rule, and the text matrix asserts all 84
  target/client combinations. See the [Employee Recruitment offer text
  audit](employee-recruitment-offer-text-audit-2026-10-28.md); this does not
  complete the Recruitment hill or provide independent Genesis pixel proof.
- The type-3 handler at `0x1F086` compares the field area/event with the
  selected offer target. On a match it selects one of the three source actor
  lists (`0x1D6D4A`, `0x1D6D5E`, or `0x1D6D72`) and resource profiles, then
  displays the entry stream at `0xFE0DE` through descriptor `0xFDE26` before
  the area event continues. Lua now gates and presents that passage only at
  the accepted offer's exact tuple. Scene 2/event 9 is thereby confirmed as a
  Halloweeners contract target; the contract data still does not prove a
  named building identity for the shared destination scene. Event-table row 9
  still contains scene 26, but pending actor descriptors make the world
  activity selector override that destination to field scene 15.
  `shadowrun_ghidra_conversion_qt_field_scene2_gang_contract_entry_qa` now
  uses ordinary Up input from the open lane south of ROM record 94, verifies
  event 9, checks the full entry text, dismisses it, then enters scene 15 and
  publishes all four ordered target actors at the positions read from the
  six-byte records at `0x16A6A`. Debug/Release output is in
  `qt_runtime/field-scene2-gang-contract-entry-qa/` (`route.tsv`, four
  full-frame BMPs). The scene-15 actor handoff is connected; combat results,
  payout, and an independent Genesis comparison remain open.
- The other two source tuples in the type-3 target table are also physically
  covered. Ordinary-Up QA reaches Puyallup Scene 1 records 76/77 (area 1/event
  9) and Penumbra Scene 3 record 100 (area 3/event 4). Exact matches show the
  same `0xFE0DE` passage and publish the corresponding four-actor Scene-15
  group; wrong-target and no-run states continue through event-table rows 9/4
  to Scene 13. Debug/Release captures for the six Puyallup/Penumbra cases are
  SHA-256 identical (26 files per build). All three target tuples are now
  physically routed through the ROM gate; fight results, payout, and
  independent Genesis comparisons remain open. See the [Puyallup entry
  audit](puyallup-ork-gang-contract-entry.md) and [Eye-Fivers entry
  audit](eye-fivers-gang-contract-entry.md).
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
- The active Qt field path calls the Lua run resolver when a wired Johnson
  interaction opens. Scene 13 uses the named Jump House context enum (0), and
  scene 14 uses Wanderer (3). The Ghoul handler completes only when the
  accepted offer's own context matches and at least one eligible Ghoul was
  counted. It returns negotiated payment times the kill count and the source
  progress delta; the field host applies those values to funds and all three
  runner records, then clears the active-run field flag. An ordinary door or
  unrelated field event cannot settle the bounty.
- On completion, Lua consumes a one-shot return result and asks the completed
  run module to present it. The Jump House text at ROM `0x105AB7..0x105B79`
  feeds the shared dialogue panel, followed by a Lua-defined follow-up menu.
  Completed-run history remains in the save registry, while this presentation
  cue is transient so a later Johnson visit does not replay it.
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
`run_ghoul_bounty.lua` receives a typed defeat fact when a live world actor's
physical meter changes from positive to zero. Lua checks actor `+0x9B == 0x42`,
increments the active offer's count up to the ROM cap of 20, and retains that
count in the ordinary run-registry save state. The projectile resolver already
publishes typed meter updates to this host path. Matching-context return,
one-time payout, and the source result dialogue now pass through the active
Jump House interaction path. The source-context mismatch and zero-kill cases
remain active and unpaid. The result screen has not received a CLI frame dump
or full-screen visual review, so this route remains partial. Escort's source
client spawn and actor state, gang's live encounter results,
recovery/recruitment item/contact callbacks, and Matrix field return-settlement
remain open.

## Visual evidence checklist

1. **Does it look correct?** Text and follow-up choice are transcribed from
   the context-zero return descriptor; no rendered result capture has been
   reviewed yet.
2. **Does visible state match behavior?** Lua requires a matching offer
   context and a positive eligible-kill count before completing. The host then
   applies the multiplied negotiated payment and source progress delta.
3. **Are the visuals correct?** Still unverified; the focused test checks the
   dialogue key and sequence, not pixels.
4. **Should it animate?** No animation was added. The result route's full ROM
   draw/timing path has not yet been independently reviewed.
5. **Is there sound?** No sound was added; the result route's audio calls and
   duration remain to be traced.
6. **Are adjacent screen elements clean?** The shared dialogue presenter
   replaces its current page, but the full-screen result capture is still open.
7. **Do multiple frames support it?** No independent longplay return frames
   have been linked to this completion sequence.
8. **Are new controls drawn over old content?** The existing shared dialogue
   and menu presenters are used, but clean panel replacement has not been
   checked in a frame dump.
9. **Was the affected time window reviewed continuously?** Not yet; a
   continuous Johnson return capture remains open.

The actor-defeat counter adds no panel, button, icon, animation, or sound call;
it only updates saved run state after the physical meter reaches zero. That
does not verify the actor's death animation or combat audio, which remain on
their separate actor/combat paths.

A new gameplay video is not needed to recover these state predicates from the
ROM. It would help verify the completion page's appearance, animation/sound
timing, and the full visible return sequence.
