# Cockpit tri-vid layer priority audit

Date: 2026-10-06  
Defect: OPEN-06, the moving cockpit skyline covered the character  
Status: resolved for the reported skyline-over-character layering

## ROM trace and native change

The startup route reaches the cockpit scene through ROM `0x5AA96`. It resets
the upload state, clears/fills both planes, installs the scene pattern streams,
draws the three story text rows, and copies the scene palette. ROM `0x5AB1C`
resets the projection and timed-sprite state and submits the initial scene
frame. The name-table stream at `0x1E620E` is then installed with attribute
`0x8000`, giving Plane A high priority.

`0x5ABD2` submits the timed, fixed, and moving sprite lists in that order:

| Source path | Sprite list | Source attribute | Typed native behavior |
|---|---:|---:|---|
| `0x5AC0E` | `0x1DF1D6` (`trivid-left`), at `(0xC2, 0x13C)` | `0xC000` | Fixed SAT list, high priority |
| `0x5AC36` | `0x1DF238` | `0x6000` | Moving exterior layer; x/y accumulators add `0x40`/`0x18` per source frame; low priority |
| `0x5AC7E` | Three lists from `0x5AD02` | `0xE000` | Timed scene sprites; wait `0x3C` from the scene start, select the next list every six source frames, then keep drawing the last selected list |

The `0x5ACF6` immediate value `-0x2000` is the 16-bit attribute `0xE000`:
palette line 3 and high sprite priority. The fixed list's `-0x4000` is
`0xC000`, also high priority. The moving exterior list is `0x6000`, low
priority. Ghidra corroborates these call values; the US ROM is the source.
There is no distinct sound request in these three list routines. The
orchestrated opening music lifecycle is documented in the resolved OPEN-01
audit; these sprite routines add no audio event.

The Qt presenter previously drew every cockpit sprite after the plane,
allowing the low-priority moving layer to cover the character. It now draws
low-priority requests before the named cockpit plane and high-priority
requests after it. The runtime retains typed projection/timer state and
individually named RGBA assets; it does not reproduce VDP planes or sprite
tables. The offline importer also used `0x2000` for the timed list, selecting
the wrong palette line. It now uses `0xE000`; the six named timed crops were
regenerated from the ROM. A source-backed assertion in `shadowrun_graphics`
guards the three table indices and attribute value.

## Full-screen evidence

The Release capture at
`build/visual-audits/intro-full/release-gator-shaman-20261006-trivid-fix/`
contains 6,782 consecutive 320x224 frames from the initial boot through the
Gator Shaman route, opening, field handoff, and 120-frame field hold. It
completed normally. Comparing every PNG by SHA-256 with the previously
reviewed full run found exactly 277 changed frames, all in the cockpit scene
(capture frames 4849-5125). The other 6,505 frames are byte-identical to that
reviewed run. `frame-compare.tsv` records old/new SHA-256 values and the
same/changed result for every capture frame.

All 277 changed whole-screen frames were reviewed in 14 labeled contact
sheets under `contact-sheets-all-trivid-frames/`. Full-size checks include
frames 4848-4850 at scene entry, 4873/4930/5000 during the sprite sequence,
5124-5127 at loop exit, and 5133-5141 through black. The adjacent transition
checks include the preceding black hold and the complete tri-vid fade-in.
The monitor stays within its green bezel, the typed text remains in place, the
pale timed detail moves across the character/console, and the exterior layer
stays behind the cockpit plane and character. No prior scene controls or
graphics remain over the cockpit, and the process reaches the field without
stalling or crashing.

The focused CTest writes full-screen 320x224 samples for scene entry, timed
phases 1-3, projection midpoint, and pre-exit; `actual.bmp` is the pre-exit
screen. Its pixel checks verify low-priority coverage behind opaque plane
pixels and visible through plane holes, plus high-priority sprite coverage.
Debug and Release CTest `actual.bmp` files are byte-identical. The test also
checks the typed phase, coordinates, and source attributes. The asset importer
test separately checks all three timed-list records retain `0xE000`.

The independent reference is
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_02_archetype_opening_story_0056-0118/`.
Frames 000820 and 000960 show black before and after the scene; 000830 shows
the reveal; 000840/000860 show the cockpit and TV; 000880/000900/000930/000940
show the changing light-colored detail around Michael and the console. These
whole-screen frames support the scene ordering, TV placement, and animation.
Their 30-fps sampling does not establish exact VBlank phase.

## Nine-question visual review

1. **Does it look correct? Yes for OPEN-06.** The skyline no longer crosses
   Michael in front of the cockpit. Independent section 02 frames 000840,
   000860, 000880, 000900, 000930, and 000940 support the cockpit composition.
2. **Does the look match behavior? Yes.** Typed requests retain the ROM
   attributes: fixed `0xC000`, moving `0x6000`, timed `0xE000`; Plane A is
   `0x8000`. The QA checks these flags and the timed/projection state.
3. **Are the visuals correct? Yes for the reported layer and palette fault.**
   The moving layer is composited behind the cockpit plate; the timed crops
   now use the source palette line 3. The monitor, text, transparency, and
   full 320x224 placement were reviewed across the whole scene. The description
   “hand/arm” is a visual reading; the ROM identifies timed SAT lists, not a
   semantic actor name.
4. **Does it animate? Yes.** The projection advances by x `0x40` and y `0x18`
   each source frame for the `0x115`-frame loop. The timed sequence starts
   after a `0x3C` source-frame gate, changes list every six source frames for
   three entries, then continues displaying the last entry through loop exit.
5. **Is there a sound?** No sound call was found in the fixed, moving, or timed
   sprite routines. The outer opening music route is separate; no sound or
   duration is attributed to these visual events.
6. **Are the other screen parts correct? Yes, reviewed.** Every changed
   full-screen frame was checked. The text, TV bezel/content, character,
   exterior layer, transition to black, and subsequent airport scene were
   inspected. The unrelated 6,505 frames of the full run are byte-identical to
   the prior reviewed capture.
7. **Do multiple frames support the interpretation? Yes.** Six source
   screenshots and 277 consecutive native frames cover the stationary cockpit,
   three timed entries, moving projection, sustained display, and exit. The
   intro has no menu click path.
8. **Does new UI replace old UI cleanly? Yes / not applicable.** This is a
   scene transition, not a child menu. The previous scene fades to black before
   the cockpit reveal, and the cockpit fades back to black before the next
   stage; no prior controls or layers persist.
9. **Was the whole time window reviewed? Yes.** The full 6,782-frame route was
   recaptured. Every changed frame was reviewed, unchanged frames were verified
   byte-identical to the reviewed full capture, and the executable reached a
   stable field hold normally.

## Outcome

- **Code changed:** Qt cockpit draw order and the offline timed-sprite palette
  attribute; six timed sprite assets regenerated.
- **Behavior implemented:** low-priority exterior sprite behind Plane A;
  high-priority fixed and timed sprites over it; timed palette line matches
  the ROM.
- **Internally checked:** Debug and Release startup, rotor, landing, wilderness
  pan, and tri-vid QA; source importer regression; six whole-screen tri-vid
  checkpoints and `actual.bmp`; complete 6,782-frame Release capture.
- **Independently visually checked:** section 02 frames 000820, 000830, 000840,
  000860, 000880, 000900, 000930, 000940, and 000960.
- **Reported defect resolved:** OPEN-06, for the moving cockpit skyline
  appearing in front of the character.
- **Still open:** OPEN-03 describes a separate death-image overlay. The
  earlier ambush/death event needs its own actor-lifetime trace and full-screen
  entry-to-exit review; this cockpit layer fix does not close it.
