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
  does not leave the screen. The check is inside the `D7 < 0x0C6C` branch, so
  Start is honored only once the prompt phase begins; a Start latched during
  the earlier logo reveal waits until that branch is reached.
- After a Start exit, `0x1A9E` takes one final sample and calls `0x131C` before
  returning. `0x131C` performs 17 palette passes. Each pass calls `0x1342`
  with a DBF count of one, so there are two VBlank waits and controller-event
  latches before each palette step: 34 VBlanks total. The title animation and
  fresh title input sampling have stopped during this transition.
- Caller `0x075E` then invokes `0x53126`. The traced service issues six fixed
  command-`0x14` transactions and waits three VBlanks after each, then flushes
  and services the audio bus: 18 VBlanks. It enters `0x59092` Start Options
  when `FFDDC0` is set. `0x59092` prepares the menu assets/font and calls
  `0x10B5A`; that helper calls `0x10918`, which resets/fills the menu display
  buffers through `0x10952` and waits for VBlank at `0x1095E`. The asset
  transfers then continue, `0x59092` writes its grid at `0x590B6`, renders the
  encoded heading and choices at `0x590CC`, and draws the cursor before the
  first input wait at `0x590EA`. The reset-to-transfer VBlank is the
  source-supported shell-only checkpoint represented by `TITLE-04`.
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

Debug and Release Qt startup QA each write 134 matching current BMPs. Every
current image is 320x224 and has the same SHA-256 in both configurations. The
exit-specific set includes the title before transition, all 17 palette-pass
frames, the completed black frame, all 18 mode-entry-service frames, and the
Start Options shell and destination. The shell-only checkpoint is also
captured after returning from the empty Load page. Each pass changes the
full-screen image only after its second VBlank. The palette has reached full
black by pass 7 and remains black for passes 8-17; this is expected after the
3-bit color components reach zero, while the source still executes every
pass. The 18 service frames remain black until the shell replaces the title.
Every tested image was reviewed at full 320x224 resolution; the shell contains
only the Start Options bezel/background, and the following page contains the
heading, default New Game selector, and choices, with no title or prior
Save/Load text behind it. CLI `actual.bmp` dumps for the shell and active page
also match between Debug and Release.

The Debug output directory also contains 16 older `seattle-outline-window`
white-outline experiment BMPs not produced by this startup CTest. They are
reviewed separately in
[`full-screenshot-review-2026-10-05.md`](full-screenshot-review-2026-10-05.md)
and are excluded from the 134-image cross-configuration comparison.

## Visual review checklist

1. **Does it look correct? — Yes for the Start transition and shell-to-text
   order.** The independent sequence contains a title fade, a black hold, two
   sampled frames of the empty Start Options shell, then the page. Native
   whole-screen output has the same sequence. The clip does not identify the
   exact controller edge or 60-Hz duration.
2. **Does the look reflect behavioral state? — Yes for the tested Start
   route.** Qt maps Space/Return/Enter to the ROM Start bit. A alone leaves the
   title running. During the fade and service hold, new input is ignored. The
   destination state selects New Game by default, matching the visible
   selector.
3. **Are the visuals themselves correct? — Yes for this slice.** The title
   fades to black; the shell phase contains only the full-screen bezel and
   background; the following page has its heading, choices, and green New Game
   arrow. Independent frames `001670`-`001674` support the order. The explicit
   ROM wait at `0x1095E` is represented as one native source-frame checkpoint.
   The clip's 30-fps sampling cannot establish its exact 60-Hz display duration
   or quantify transfer work between waits.
4. **Should anything animate? — Yes during the title exit; no recurring
   animation is identified in the shell/page transition.** Source performs 17
   palette steps, one every two VBlanks. The title animation and fresh input
   sampling pause. The caller then holds black while six audio-service
   transactions each wait three VBlanks. The menu preparation has an explicit
   shell-only VBlank at `0x1095E`; the typed host shell lasts through one source
   frame and advances to the static menu page.
5. **Is there sound? — No new call is traced between the shell checkpoint and
   menu input; the preceding caller audio request remains unresolved.**
   `0x131C` has no direct sound request. Caller `0x53126` issues six command-
   `0x14` transactions, but the resource alias, audible start/stop, measured
   duration, and output-device playback have not been verified. The Qt QA
   records service boundaries only; it does not claim the original is silent.
6. **Are the rest of the screen and related elements correct? — Reviewed.**
   All 134 whole-screen startup QA images were scanned, including title and
   boot states, every fade/service checkpoint, opening transition, shell,
   active menus, and the empty-save return. The title clears before the shell
   appears; the no-save message clears before the shell is redrawn.
7. **Do multiple frames support this interpretation? — Yes, with limits.**
   Native QA writes every pass and every service VBlank. Independent frames
   `001637`-`001643` and `001670`-`001674` support the sequence across multiple
   samples. The capture has one Start-menu opening and no input/VBlank log, so
   it cannot establish exact 60-Hz cadence or show a second input route.
8. **Does the new UI replace the old UI cleanly? — Yes.** The title reaches
   full black before the shell appears. During the shell checkpoint, no menu
   text or cursor is drawn; the next full redraw shows only Start Options.
   Returning from the empty Load result clears that message before the shell
   and then the menu return.
9. **Was the complete screenshot sequence reviewed? — Yes.** The six contact
   sheets cover every current startup QA BMP; key menu, return, and intro
   frames were checked individually so selector and overdraw issues remain
   legible at native artifact resolution.

## Verification and outcome

- **Code changed:** `TitleExitPaletteFadeState` models the typed 17-pass
  transition; `TitleModeEntryServiceState` models the six transaction/18
  VBlank caller service; `StartupIntegrationPhase::start_options_shell`
  models the shell-only checkpoint and reveal. Qt forwards Start to the ROM
  Start latch, the converted loop now checks that latch only when the prompt
  schedule enters the ROM's `D7 < 0x0C6C` branch, and input stays held during
  the source transition.
- **Behavior implemented:** Start exits the title, runs all 34 transition
  VBlanks, holds black for the 18 caller-service VBlanks, shows the shell
  checkpoint, then opens Start Options with New Game selected. A alone does
  not exit; Start latched before the title prompt waits until prompt-phase
  input is accepted. A button held through the shell is sampled when the menu
  becomes active. Empty Load return repeats the shell then reveals the menu.
- **Internally checked:**
  `shadowrun_ghidra_conversion_qt_startup_qa` and
  `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`
  passed in Debug and Release; the full Qt CTest suite passed 53/53 in both
  configurations. The typed `shadowrun_ghidra_conversion` CTest also passed in
  Debug and Release. The 134 current startup images match across
  configurations and have 320x224 BMP dimensions. All were scanned as contact
  sheets; 17 relevant menu/route checkpoints were individually reviewed at
  full size for the Lua key-binding slice.
  The title QA asserts all 17 palette steps, no early destination entry, all
  six transactions, all 18 service waits, black hold, menu selection, and clean
  full-frame replacement.
- **Independently visually checked:** section 01 frames `001637`-`001643` and
  `001670`-`001674` show title-to-black-to-options order. They do not log the
  Start edge or VBlank phase.
- **Reported defect resolved:** `TITLE-02` is resolved for the reported Start
  route. The source-supported shell-to-text ordering in `TITLE-04` is also
  implemented and defended; the capture cannot establish exact 60-Hz display
  duration. Idle-timeout continuation and command-`0x14` audio identity and
  playback remain open.

## Startup menu text bindings (2026-10-09)

`startup.lua` now returns explicit title and per-choice text keys for Options,
Save/Load slots, and archetype selection. The runtime validates the optional
title and ordered label list against the returned stable choice IDs. Cursor
state, page transitions, and rendering remain in the native startup
controller. The ROM-derived wording remains in the keyed Lua text resource at
`assets/dialogue/script-text.json`; this change binds each menu entry to that
source copy instead of duplicating text or constructing key names in C++.

For this content-only change, appearance, current selection, selector and
portrait details, related screen regions, and clean full-panel replacement
were reviewed across the complete screenshot set. The startup pages remain
static; text-key binding does not change animation cadence or sound behavior.
The independent 30-fps capture supports the multi-frame title-to-menu order,
but does not establish menu sound duration or every 60-Hz animation state. All
134 Debug and Release QA images have matching hashes; all were scanned as
contact sheets and 17 menu/route checkpoints were inspected individually at
full size. This is internal consistency evidence, not independent proof of
Genesis visual fidelity for every choice route.

## Archetype portrait selection through Lua (2026-10-09)

`startup.lua` now maps the stable `samurai`, `decker`, and `gator_shaman`
choice IDs to their individually extracted portrait aliases. The native
startup selection remains authoritative; rendering looks up the alias for
that selected choice. The ROM path at `0x592C2` selects the portrait resource
and calls `0x1F38A`; the extracted choices are the three entries at `0x7291A`.
The visible archetype page contains a portrait and choices, but no stat rows.
Starting attributes continue to come from the selected ROM-derived runner
record, so this presentation binding does not invent or duplicate stat data.

The visual review used the seven-question anomaly pass:

1. The three full-screen native states show Samurai, Decker, and Gator Shaman
   portraits with the cursor on the matching row. Independent section 02
   frames `000030`, `000045`, and `000060` show the same three pairings.
2. Selection behavior remains native input state; Lua supplies only the
   portrait alias keyed by the selected stable choice ID. Script-runtime QA
   checks all three ID/alias pairs and startup QA exercises all three choices.
3. The portrait stays at the traced `(64,64)` position. The shell, heading,
   choice spacing, cursor, and other screen regions are unchanged.
4. Portraits are static. A portrait changes only when the archetype selection
   changes; this slice adds no animation or cadence.
5. This slice adds no sound request and changes no startup audio behavior.
6. Start Options, both Save/Load states, the return page, and all three
   archetype states were checked as complete screens. The old page's controls
   do not remain over the new page.
7. Independent frames `000030`, `000045`, and `000060` show the selector
   moving through each portrait. Native frames `08_archetype_samurai.bmp`,
   `09_archetype_decker.bmp`, and `10_archetype_gator_shaman.bmp` cover the
   same states.

Fresh Debug and Release startup captures each contain 134 BMPs. All 134
hashes match between configurations and the previously reviewed capture; no
pixel changed in this asset-selection-only refactor. The screen images verify
the host integration and match the three observed independent choice states;
they do not establish Genesis fidelity for unobserved input paths or audio.
