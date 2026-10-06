# Title Start-to-Options Transition Audit

Date: 2026-10-06  
Scope: `TITLE-02` reported skip and `TITLE-04` shell-to-text timing found in
the same whole-screen review.

## ROM path

The primary source is the US ROM trace at `0x1A9E`, `0x0BF2`, `0x131C`,
`0x1342`, and the caller at `0x075E`. Ghidra C and the exported disassembly
were used only as cross-checks.

- The title loop samples each update at `0x195C`, records newly pressed
  controller bits at `0x1BEA`, and calls `0x0BF2`. That latch marks general
  activity in `FFDDC0`; it sets the title exit latch `FFDDC2` only when
  controller bit 15 is pressed. The title loop checks `FFDDC2`, so A by itself
  does not leave the screen.
- After a Start exit, `0x1A9E` takes one final sample and calls `0x131C` before
  returning. `0x131C` performs 17 palette passes. Each pass calls `0x1342`
  with a DBF count of one, so there are two VBlank waits and controller-event
  latches before each palette step: 34 VBlanks total. The title animation and
  fresh title input sampling have stopped during this transition.
- Caller `0x075E` then invokes `0x53126`. The traced service issues six fixed
  command-`0x14` transactions and waits three VBlanks after each, then flushes
  and services the audio bus: 18 VBlanks. It enters `0x59092` Start Options
  when `FFDDC0` is set. `0x59092` prepares the menu assets/font, writes the
  menu grid, and renders the encoded heading and choices before its controller
  loop.
- A natural title-counter timeout follows a different caller branch after
  `0x53126` (`0x078A` onward). This slice does not claim that idle continuation
  is reconstructed.

## Full-screen and independent frames

Independent reference: the numbered 30-fps section 01 capture, full
320x224 frames. Frames `001637` and `001638` show the settled title;
`001639` through `001642` show it darkening; `001643` through `001669` are
black. The Start Options shell appears at `001670` and `001671`; its heading,
New Game/Load Game text, and selector are visible at `001672` through `001674`.
The recording contains no controller input log or Genesis VBlank counter, so
it establishes the visual sequence, not the exact point of the Start edge or
60-Hz phase.

Debug and Release Qt startup QA each write 125 matching current BMPs. Every
current image is 320x224 and has the same SHA-256 in both configurations. The
exit-specific set includes the title before transition, all 17 palette-pass
frames, the completed black frame, all 18 mode-entry-service frames, and the
Start Options destination. Each pass changes the full-screen image only after
its second VBlank. The palette has reached full black by pass 7 and remains
black for passes 8-17; this is expected after the 3-bit color components reach
zero, while the source still executes every pass. The 18 service frames remain
black until the menu replaces the title. Every tested image was reviewed at
full 320x224 resolution; the destination contains the Start Options shell,
heading, default New Game selector, and choices, with no title pixels behind
it.

The Debug output directory also contains 16 older `seattle-outline-window`
white-outline experiment BMPs not produced by this startup CTest. They are
reviewed separately in
[`full-screenshot-review-2026-10-05.md`](full-screenshot-review-2026-10-05.md)
and are excluded from the 125-image cross-configuration comparison.

## Eight-question visual review

1. **Does it look correct? — Yes for the reported skipped-transition
   symptom.** The independent sequence contains a title fade, a black hold,
   then Start Options. Native whole-screen output now has the same order. The
   clip does not identify the exact controller edge or VBlank phase.
2. **Does the look reflect behavioral state? — Yes for the tested Start
   route.** Qt maps Space/Return/Enter to the ROM Start bit. A alone leaves the
   title running. During the fade and service hold, new input is ignored. The
   destination state selects New Game by default, matching the visible
   selector.
3. **Are the visuals themselves correct? — Partly.** The title fades to a
   clean black screen; the destination uses the full-screen Start Options
   panel and has no stale title art. Independent frames show two shell-only
   samples before menu text appears. Qt currently draws the complete menu at
   once, so that separate presentation difference remains `TITLE-04`. The
   reference does not establish its source-timed duration.
4. **Should anything animate? — Yes, during the exit only.** Source performs
   17 palette steps, one every two VBlanks. The title animation and fresh
   input sampling pause. The caller then holds black while six audio-service
   transactions each wait three VBlanks. The menu has no transition animation
   identified in `0x59092`; its shell/text display interval is under
   investigation.
5. **Is there sound? — A caller audio request is present; playback is
   unresolved.** `0x131C` has no direct sound request. Caller `0x53126` issues
   six command-`0x14` transactions, but the resource alias, audible start/stop,
   measured duration, and output-device playback have not been verified. The
   Qt QA records service boundaries only; it does not claim the original is
   silent.
6. **Are the rest of the screen and related elements correct? — Reviewed.**
   The review covered the full title, every fade checkpoint, black hold,
   shell/text reference sequence, and the full destination. The title clears
   before Start Options appears. The exact shell-to-text timing remains open.
7. **Do multiple frames support this interpretation? — Yes, with limits.**
   Native QA writes every pass and every service VBlank. Independent frames
   `001637`-`001643` and `001670`-`001674` support the order across multiple
   samples. No input/VBlank log is available, and 30-fps video cannot establish
   exact 60-Hz cadence.
8. **Does the new UI replace the old UI cleanly? — Yes for the completed
   destination.** The title reaches full black before the Start Options page
   is drawn. The full frame shows the new shell and choices without the old
   title. The reference's earlier shell-only stage is not yet represented in
   Qt and remains separately tracked as `TITLE-04`.

## Verification and outcome

- **Code changed:** `TitleExitPaletteFadeState` models the typed 17-pass
  transition; `TitleModeEntryServiceState` models the six transaction/18
  VBlank caller service. Qt forwards Start to the ROM Start latch and holds
  input during the source transition.
- **Behavior implemented:** Start exits the title, runs all 34 transition
  VBlanks, holds black for the 18 caller-service VBlanks, then opens Start
  Options with New Game selected. A alone does not exit.
- **Internally checked:**
  `shadowrun_ghidra_conversion_qt_startup_qa` and
  `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`
  passed in Debug and Release; the full Qt CTest suite passed 53/53 in both
  configurations. The typed `shadowrun_ghidra_conversion` CTest also passed in
  Debug and Release. The 125 current startup images match across
  configurations, have 320x224 BMP dimensions, and were reviewed at full size.
  The title QA asserts all 17 palette steps, no early destination entry, all
  six transactions, all 18 service waits, black hold, menu selection, and clean
  full-frame replacement.
- **Independently visually checked:** section 01 frames `001637`-`001643` and
  `001670`-`001674` show title-to-black-to-options order. They do not log the
  Start edge or VBlank phase.
- **Reported defect resolved:** yes for the reported Start transition being
  skipped. The exact timing, shell-to-text phase (`TITLE-04`), idle-timeout
  continuation, and command-`0x14` audio identity/playback remain open.
