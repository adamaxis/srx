# Shadowrun visual and gameplay defect goals

This is the goal tree for the user's itemized defect report in the 2026-10-05
conversation. A user report creates a goal to investigate and solve; it does
not by itself prove that the current build still reproduces the defect. Each
goal therefore keeps these labels separate: reported, reproduced in the
current build, implemented, independently checked, and resolved.

## Completion gate for every goal

Before changing a screen or behavior, trace the exact US ROM path: entry,
controller/cursor and RAM state, animation and draw sequence, sound request,
and exit/return path. Use Ghidra as a cross-check. Use the numbered independent
gameplay breakdown under `video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/`
as the visual/audio reference; cite frame numbers and review whole 320x224
frames. A missing matching reference stays an explicit limitation.

Every completed screen/mechanism goal must have typed native state, a
regression CTest, full-screen CLI dumps of entry/active/idle/confirm/cancel/
return states as applicable, and Debug and Release validation. Review every
dump across the entire screen, not just the edited control. A child page must
replace or hide its parent controls before drawing. For animated elements,
record the counter, cadence, loop duration, and start/pause/stop conditions.
For audio, identify the ROM event/resource, verify start and stop behavior,
and record measured duration; an alias lookup alone is not playback evidence.

Apply the project's eight questions from
[`visual-verification-checklist.md`](visual-verification-checklist.md) to
each changed slice. Do not mark a reported defect resolved from a same-source
compositor, internal screenshot, or passing test alone. Record absent
independent references as absent.

## Itemized report and atomic goals

The rows below preserve every distinct issue in the user's pasted report.
`Reported` means the player observed it; it is not a claim that the current
checkout still reproduces it. Current-build reproduction and independent
Genesis comparison remain separate evidence fields. Detailed ROM paths and
shared acceptance gates are in the matching goal cluster below.
`TITLE-04` is an additional discrepancy found during the current full-screen
review; it is not part of the pasted report.

| Goal | Reported issue, itemized | Evidence check and closure condition |
|---|---|---|
| `BOOT-01` | The shock figure has no sound when it shocks the SEGA logo, and the same effect plays at the wrong time when the Decker intro starts. | Trace both ROM sound requests and runtime triggers separately; capture the logo shock and Decker-scene entry; verify playback start, duration, and stop on the output device. |
| `BOOT-02` | The flame decal remains on the right after the logo is fully revealed. | Reproduced in the pre-fix Qt full-screen title capture. The typed reveal stops at 38 columns: the last active frame shows the flame at title frame 165, the first frame after the 0x1A9E reveal branch hides it at 166, and settled frame 900 has no right-edge overlay. Debug/Release CTest checks projection state and whole-screen right-edge pixels. Independent section 01 frames 000372, 000384, and 000390 show the same active-then-absent transition. Resolved for the reported post-reveal persistence. |
| `BOOT-03` | The animated microchip/circuit background does not move upward. | Independent section 01 frames 000390, 000396, 000420, and 000450 show changing background positions. Trace source offset, direction, cadence, and lifetime; compare a multi-frame full-screen dump. |
| `TITLE-01` | The title's “PRESS START” label does not blink in and out. | Reproduced before the edit: the presenter set `title_prompt_visible_ = true` for both ROM variants. Resolved. ROM `0x1A9E` starts D7 at `0x0E10`, waits once per loop, then draws only when D7 is below `0x0C6C`; its low six bits select visible record A (`0x1F81`) or blank record B (`0x1F99`), both at `0xCA84` with palette `0x2000`. Qt now follows the selected typed text asset. Debug/Release typed and full-screen QA cover two cycles. Independent section 01 frames 000900/000906/000912 show visible/hidden/visible; their coarse 30-fps spacing supports blinking but does not establish absolute VBlank phase. Full review: [title prompt blink audit](visual-audits/title-prompt-blink.md). |
| `TITLE-02` | Pressing Start skips the transition and immediately opens New Game/Load Game. | Resolved for the reported Start route: ROM `0x1A9E` exits only on the Start latch (`0x8000`), then performs 17 palette passes after 34 VBlanks; the `0x075E` caller runs six command-`0x14` audio transactions over 18 more VBlanks before `0x59092` draws Start Options. Debug/Release QA captures every pass and hold frame, proves A alone does not exit, and checks the default New Game selection and clean destination redraw. Independent section 01 frames 001637-001643 show title-to-black and 001670-001674 show the options shell followed by text. The clip has no input/VBlank log; exact phase and the shell-to-text reveal remain open under `TITLE-04`. Audio resource/playback/duration also remain unidentified. See [title Start transition audit](visual-audits/title-start-transition.md). |
| `TITLE-03` | The title circuit background is much brighter and denser green in the native screen than in the independent game capture. | Reproduced in matched whole-screen comparisons after correcting a mislabeled native frame. ROM tracing supports the extracted base palette and identifies a separate seven-phase light update; the compressed reference does not establish the correct brightness of thin circuit lines. Keep open until source-backed color/texture equivalence can be established without a speculative color transform. See [title circuit palette audit](visual-audits/title-circuit-palette.md). |
| `TITLE-04` | Found during full-screen verification: the Genesis capture shows the Start Options shell without text for frames 001670-001671, then shows the text and selector from 001672; the current Qt destination appears complete immediately. | Trace `0x59092` grid write (`0x590B6`), text write (`0x590CC`), and first explicit menu VBlank wait (`0x590EA`), plus `0x10B5A` asset transfer timing. Reproduce only source-supported staging, capture each full-screen phase, and verify the selector and old-title clearing. The 30-fps reference shows the order but not exact VBlank timing. See [title Start transition audit](visual-audits/title-start-transition.md). |
| `OPEN-01` | Opening music starts on the second screen instead of when the opening starts. | Trace each timed opening stage's sound command and playback state; compare section 02 audio/frame timing and record duration/loop/stop behavior. |
| `OPEN-02` | In the Decker route, the head floats instead of staying attached to the character. | Identify the exact scene, actor record, and layer ownership; compare several adjacent frames and full-screen native captures. The sampled section 02 frames do not yet identify this Decker-specific state. |
| `OPEN-03` | In the Decker route, the body is superimposed over another image. | Trace scene handoff and layer clearing independently from head placement; verify no old actor/background layer remains across the full frame. |
| `OPEN-04` | The aircraft's rear propeller does not move. | Trace its frame list, cadence, and active-scene conditions; compare a continuous independent sequence and multi-frame full-screen dump. |
| `OPEN-05` | The aircraft lands on the building incorrectly. | Trace landing position, orientation, and layer order through the landing transition; review approach, contact, and settled frames. |
| `TAXI-01` | The taxi-window callout is missing its top/bottom edges and appears as two parallel side lines instead of a closed white outline. | Trace the source frame/layout and redraw ownership; compare full taxi frames before and after the callout. |
| `HUD-01` | The field portrait has unexpected color over the runner's face. | Resolved for the reported Gator face-color symptom: ROM `0x35D4` applies `+0x2C` to the 4x4 face; the old Qt path used `0x3602`. Corrected assets/runtime and per-record QA select Gator line 1 (`+0x2C=0x2000`); independent section 04/06 frames support that palette. See [the eight-question portrait audit](visual-audits/hud-portrait-palette.md). Ammo remains separately tracked as `HUD-02`. |
| `HUD-02` | Ammunition is inconsistent: the report cites 32 bullets initially, 37 on the unequipped hand, and 4 after equipping the gun. | ROM Gator record `0x72B1A` starts at 32 with item code 2; the `0x39F2` field refresh reaches `0x36FC`, which clamps it to the item-2 capacity 12 at `0xD4570 + 0x24`. The Qt path applies that refresh. ROM `0x125FA`/`0x121E0` plus item code 3 capacity 11 reproduce `4/11 -> 37 -> 4`, but the ordinary unarmed field-HUD branch draws `00` while preserving the +0x4E percentage. The user's visible 37-on-hand route remains unreconciled. The former Ricky QA image used an impossible raw-template/unarmed state and is corrected to use a refreshed live record and typed unequip. See [ammo capacity audit](visual-audits/hud-ammo-capacity-refresh.md); keep the reported sequence partial until independently reconciled. |
| `FIELD-01` | Walking uses the dying/fallen animation. | ROM trace identifies C062 as the ordinary four-pose movement loop and C052 as idle; C0B2 is a separate finite behavior transition. Verify all eight selector/assets, live movement and cadence, blocked reset, release, and full-screen states. |
| `SOUND-01` | Pocket Secretary selection/cancel sounds are absent on some screens or use the wrong sound. | Audit each source action and child page separately; verify the event, trigger condition, measured duration, and playback on the default output. |
| `SOUND-02` | Moving between Pocket Secretary menu items has no navigation sound. | Trace the accepted cursor-move edge and source resource; verify one playback per accepted move, none on clamp/no-op, and the stop/duration behavior. |
| `SEC-MAGIC` | The Magic page leaves its old text/menu content drawn over the new page. | Trace page entry/exit and panel clear/redraw; capture parent, child, return, and reopen at full screen, proving old controls appear only when the source restores them. |
| `SEC-CYBERWARE` | Opening Cyberware hides inventory, the runner portrait, posture, and text that should remain visible. | Identify which HUD/panel regions are shared; compare complete page frames and verify child entry/return redraws shared regions exactly once. |
| `KARMA-COLOR` | The Start-menu Attributes/Skills Karma page uses the wrong selection color. | Keep this view-only page separate from Hotel Karma spending; trace the selected row's palette and compare every selectable row against source/reference evidence. |
| `KARMA-DETAIL` | The Start-menu Attributes/Skills Karma page omits the selected attribute/skill's detail text. | Trace row-to-detail data and placement; verify each row and confirm hover alone neither opens the page nor spends Karma. |
| `CLIPS-REDRAW` | The Clips menu is drawn over its parent menu instead of replacing it cleanly. | Trace the source panel replacement/restore path; compare parent, child, confirm/cancel, and return full-screen captures, including all buttons behind the child. |
| `POSTURE-FOCUS` | The posture bar has no selector; it should be reachable with Up/Down from the menu buttons and affect attack/defense. | Existing ROM/source and internal QA describe Up focus, Down deselection, five posture states, and attack/defense updates. Reproduce through normal play and reconcile the report with the full-screen QA capture that currently shows a selector. |
| `INVENTORY-01` | Moving/trading items between inventory locations does not work. | Trace source/destination cell state, item constraints, confirmation/cancel, and authoritative item records; exercise the route through ordinary controls and verify both cells and stats. |
| `INVENTORY-02` | Using an item draws the runner portrait over other bars. | Trace item-use page/result/return and full redraw order; verify portrait, bars, posture, message, and inventory across all transition frames. |
| `ITEMS-01` | Armor, weapons, and their damage values are wrong. | Reconcile named item assets and source catalog values with equip state, attack/damage calculations, and inventory/shop views; compare multiple weapons/armor states. |
| `FIELD-02` | The runner gets stuck in terrain and cannot enter buildings. | Trace movement probes, collision margins, terrain gates, physical building records, event publication, and exits; verify contact and adjacent walkable cells through ordinary D-pad input. |
| `STOKER-01` | The physical Stoker collision/property that should open his generic dialogue is still unidentified. | Resolved in the normal New Game route: scene-2 record 91 at `0x173138` (`0xC2`) is reached with Up, publishes raw collision `0xFFC2` and event 2, then mode 2/event 2 selects the generic Stoker arrival panel. Debug/Release CTest, full-screen route dumps, eight-question review, and independent section 03 frames support the result. The clip confirms the door/fade/panel sequence but does not record input or event bytes. See [`visual-audits/motel-doorway-scene-entry.md`](visual-audits/motel-doorway-scene-entry.md). |
| `PLAY-01` | Shops, building item use, and maglock passkeys cannot be tested from normal Qt gameplay. | Provide source-backed, normal-control routes from field play to each named feature and back; test them end to end without injecting event tuples. |
| `SOUND-03` | More generally, sound effects across the game are reported to play at the wrong points. | Inventory the reported and currently connected ROM sound requests, map each to a state transition, and check trigger, playback, duration, loop, and stop. Close individual sounds in their owning goals. |

### Earlier reported items carried forward

| Goal | Reported issue | Closure condition |
|---|---|---|
| `EQUIP-01` | The yellow item selector was previously reported as a thick rectangular mess instead of the game's slim yellow outline. | The current Qt frames contain exactly 76 `#C4BE27` pixels: a one-pixel perimeter around the 24x16 item image, with no fill. Independent section 06 frames 002560/002580/002610/002640 support the same slim marker. The debug-only 4x crop scales that one pixel to four display pixels. Current geometry is not reproducing the reported thick marker; see [the selector and redraw review](visual-audits/item-selector-and-unequip-review.md). |
| `EQUIP-02` | A black box can remain around an item icon after equipping and unequipping it. | Exercise repeated equip/unequip and replacement; prove the icon cell restores its correct background and no stale border/graphic remains. |
| `LOADOUT-01` | Weapon clips/ammunition were previously reported as not being tied to the weapons. | ROM uses each item's `+0x24` capacity, runner `+0x4E` for loaded rounds/unequipped percentage, and one runner-wide `+0x4F` Universal Gun Clips reserve. Two weapon capacities and reload consumption are covered internally; normal combat fire/reload and an independent loaded-save transfer remain open. Do not add a per-weapon reserve unsupported by the ROM. |

### Evidence gathered for this pass

- Independent section 01 frames were inspected in sequence. They support the
  flame lifetime, changing title circuit geometry, copyright appearance, and
  visible/hidden Push Start phases. The title-to-options transition is cut out
  of the video.
- A frame-order bug invalidated the old comparison labeled `actual.bmp`:
  prompt QA had already advanced the title to frame 530 before the checkpoint
  labeled frame 300. The startup QA now captures the title reference sequence
  first, writes `title_300.bmp` at actual frame 300, and records title frame,
  scroll offset, and circuit phase in `title-scroll-qa.tsv`. Debug and Release
  output for `actual.bmp` and `title_300.bmp` is byte-identical.
- The independent section 01 copyright first appears at video frame 418;
  the ROM title loop first draws it at title frame 254. This observed event is
  used only as an alignment anchor, not as a claim that video frame zero is a
  ROM VBlank. Candidate whole-screen pairs are 390/198, 396/210, 418/254,
  420/258, 421/260, 422/262, 423/264, 424/266, 425/268, and 450/318
  (video/title frames). Background geometry aligns at the same vertical
  offset in these pairs, within one pixel on frame 424. The compressed video
  still makes thin-line color an uncertain measure.
- The title circuit remains visibly brighter and denser green in Qt than in
  the independent frames after timing and scroll alignment. The ROM palette
  trace supports the base asset colors and the known palette writer changes
  only its separate light phase. The display difference is recorded as
  `TITLE-03` and left unresolved rather than applying an unsupported
  brightness transform. See the [full title circuit audit](visual-audits/title-circuit-palette.md).
- BOOT-02 was reproduced in the pre-fix full-screen Qt title capture. The
  source-derived flame projection is visible through title frame 165
  (`FFF1E6` reaches `0x26` after the final column), hidden on frame 166 when
  `0x1A9E` takes the `0x0E34` reset path, and remains absent at settled frame
  900. Debug/Release full-screen dumps match by SHA-256. Independent section
  01 frame 000372 shows the transient flame; frames 000384 and 000390 show
  it gone. The larger circuit brightness/texture discrepancy remains open as
  `TITLE-03`.
- The Qt pre-title audio callback records typed `0x16` / `0x5C` requests but
  does not call the audio player; neither command has a verified alias in the
  current playback map. Their shock identity, relationship to the Decker
  route, and durations remain open under `BOOT-01`.
- Independent section 02 samples show Start Options, archetype selection, and
  later opening-story scenes. They do not yet isolate the reported Decker
  head/body, propeller, landing, or taxi states.
- The current Debug posture QA's full-screen `page-2-up-focused.bmp` visibly
  contains the `POSTURE` selector and bar. That disagrees with the report from
  normal play; the menu goal remains open until the normal route is reproduced
  and compared.
- Stoker's scene-2 trigger is now identified as terrain record 91 at ROM
  `0x173138` (`0xC2`, bounds `0x3C0..0x3DF` by `0x280..0x29F`). New Game starts
  with FFDF40 clear. Ordinary Up reaches its expanded contact margin, publishes
  raw collision `0xFFC2` and event 2, and selects the Stoker arrival record
  through mode 2/event 2. A advances `< MORE >` to the manager, B selects the
  leave response, and A returns to a clean scene-2 field. The unrelated record
  2 candidate at `0x172DBE` still produces event 0 and remains rejected. The
  physical route is Debug/Release covered; independent section 03 frames
  `000028..000054`, `000055`, and `000080..000082` support the doorway, fade,
  blank panel, and populated Stoker panel, but do not log input or event state.
  The native capture goes from full black to the populated panel; exact timing
  for the two sampled blank-panel frames remains unresolved.
- FIELD-01 now uses separately named C062 movement assets and typed selector/
  clock state. Debug and Release field-movement QA passes: all eight selector
  pairs, a live right-held route, five frame advances and a loop wrap over 35
  updates, the source displacement at tick 16, and release-to-idle are checked.
  Thirteen whole-screen frames were reviewed; the Debug/Release SHA-256 sets
  match. The scene-2 wall route also verifies a normal blocked step returns to
  C052/frame-zero idle; its 22 full-screen Debug/Release frames match and were
  reviewed. Independent section 04 frames 000900-000921 support a moving
  runner but show another map and contain no controller/selector log. Other
  live headings, exact placement, and movement sound remain open.

## Goals

### INTRO-01 — SEGA shock and logo reveal (`BOOT-01` through `BOOT-03`)

**Reported:** the shock sound is missing at the logo shock and plays at the
wrong intro point; the flame decal remains after the logo is fully revealed;
the chip background should scroll upward but does not.

**Solve:** trace the reveal/effect layers and sound requests in the ROM; remove
the decal at the source transition; implement the chip scroll with the source
direction, distance, cadence, and screen lifetime; start the correct shock
resource at the shock event and measure its duration.

**Done when:** multi-frame whole-screen dumps show shock, decal removal, and
upward scroll at the source-derived times; an independent section 01 sequence
supports the visual order; audio playback is associated with the shock rather
than startup and its duration/stop are recorded.

**Status:** partial. `BOOT-02`'s post-reveal flame persistence is resolved with
source-state retirement, a Debug/Release full-screen pixel check, and the
independent frame 000372-to-000384 transition. `BOOT-03`'s source-cadenced
upward circuit scroll is implemented and covered by the Debug/Release startup
QA. `BOOT-01` remains open; the full title still has the separately tracked
palette finding below.

### INTRO-02 — Push Start and title-to-start transition (`TITLE-01`, `TITLE-02`)

**Reported:** Push Start does not blink, and Start skips the transition into
New Game/Load Game.

**Solve:** identify the source blink counter/visibility interval and the
Start-edge transition/fade or wipe, then bind New Game/Load Game input only
after the transition reaches its destination state.

**TITLE-01 result:** the Push Start blink is resolved. ROM-derived typed state
selects a visible 34-frame opening phase, then blank for 10 frames, followed by
54 visible and 10 blank frames per 64-frame cycle. Five Debug/Release full
screens check visible/blank/visible across two cycles and assert that the
copyright row survives. Independent section 01 frames 000900, 000906, and
000912 support the blink, but not its exact VBlank alignment. See the
[eight-question audit](visual-audits/title-prompt-blink.md).

**Done when:** dumps include visible/hidden Push Start phases, Start pressed
and released, every transition phase, and the clean destination menu. The
independent section 01 frames support the blink phases; the title-to-options
transition is not shown in the supplied video and stays source-verified only
unless another matching capture is found.

**Status:** partial. `TITLE-01` is resolved. `TITLE-02` remains pending: the
independent video cuts at the title-to-options transition, and its source
transition timing still needs full-screen entry/phase/destination QA.

### TITLE-03 — Circuit background color and texture (added by full-screen audit)

**Observed:** the full-screen Qt title shows substantially more bright green
circuit detail than the independent title frames. Earlier pixel statistics
used a mislabeled Qt frame and unaligned reference frames, so those values are
withdrawn as comparison evidence. Correctly labeled whole-screen pairs now
align by the observed copyright transition and circuit geometry, and still
show a mismatch; video compression remains a limit on thin-line color.

**Solve:** trace the ROM background palette and seven light phases, inspect
the offline extracted assets, and compare full frames at matched scroll and
light states. Correct the native palette/resource mapping only when the ROM
and independent frames support the change. Do not derive a color transform
from compressed video statistics alone.

**Done when:** consecutive independent title frames and whole-screen Qt dumps
show the same circuit geometry, brightness range, and phase behavior while
the logo, prompt, and copyright remain in the correct layers.

**Status:** partial. The ROM palette and phase writers are traced, and the
frame labels and event-based reference alignment are corrected. The native
screens remain brighter and denser green than the independent capture. ROM
palette evidence supports the current extracted base colors, while video
compression prevents a confident colorimetric correction. Keep the visual
defect unresolved. See the [eight-question audit](visual-audits/title-circuit-palette.md).

### INTRO-03 — Decker, aircraft, landing, and taxi frame (`OPEN-01` through `OPEN-05`, `TAXI-01`)

**Reported:** intro music starts late; a Decker head floats and a body is
superimposed; the rear propeller is static; the aircraft lands incorrectly;
the taxi window is drawn as two side lines instead of a complete white
outline.

**Solve:** verify scene and layer ownership for the head/body, start music at
the source scene entry, identify propeller frame/cadence, trace the landing
position/orientation, and redraw the taxi window as the source-supported
closed outline. Review the full frame for old layers surviving each handoff.

**Done when:** source-derived layer state and landing sequence are represented
as typed scene state, multi-frame dumps cover propeller motion and landing,
matching independent section 02 frames support placement/order, and music
timing is verified from the source request through playback. If the Decker
route is absent from the supplied recording, record that limitation.

**Status:** pending.

### FIELD-01 — Field actor movement pose and animation

**Reported:** held movement appears to use a dying/fallen pose.

**Current source finding:** the previous Qt runtime selected its extracted
`0xC0B2` family on every accepted movement. ROM `0xB6DE` instead installs a
direction selector from `0xC062` after an accepted movement step; its positive
four-frame list loops. `0xB442` restores the directional idle selector from
`0xC052`. `0xC0B2` is an eight-descriptor finite behavior transition selected
by `0xB330`/`0xAFF2`, which explains the fallen final pose when it was used as
the walk cycle. For the kind-3 runner, `0xBABC` writes `0x28` to +0xCA;
`0x600E` sets +0x18 to `0x280` against the `0x1000` period, so the pose steps
at six- or seven-update intervals depending on phase. The section 04 recording
shows multiple field movement poses but has no synchronized input or selector
log.

**Implementation:** the Qt runtime now consumes separately named C062 frames
for ordinary accepted movement and C052 frames for idle/reset. Selector,
direction, frame index, accumulator, and period remain typed native state;
C0B2 stays a distinct finite behavior transition. The Qt CTest checks all
eight selector pairs, all four extracted frames, the live right-held path,
cadence/loop wrap, movement position, and release-to-idle. Debug and Release
full-screen output is byte-identical and has been inspected across the whole
screen. The independent corridor sequence supports changing original-game
movement poses but does not show this same map or input state.

**Remaining:** exercise the other seven headings as live field routes. An
existing normal-input scene-2 wall route now checks that blocked movement
returns to C052 with frame zero in Debug and Release. Find an independent
same-scene comparison for exact placement if available; keep that limitation
explicit if the corpus has no match. Movement audio is a separate open
question.

**Done when:** ROM selector and cadence are documented; the Qt runtime consumes
the corresponding named assets directly; normal input exercises all eight
headings, cadence, blocked reset, and release; Debug/Release full-screen dumps
pass review; independent frames support the pose and exact-scene limits are
recorded. The separate building/property routes do not get folded into this
visual claim.

**Status:** partial. The source-backed C062 selector/assets/clock correction
is implemented and passes Debug/Release for a normal right-held route; all
eight selector mappings and extracted movement families are checked. A
separate normal-input wall route verifies blocked reset. The reported defect
remains open until other live headings and exact-scene visual comparison are
covered. Independent video supports movement poses but cannot identify the
input/selector or exact map placement.

### FIELD-02 — Terrain collision, building access, and physical Stoker trigger

**Reported:** movement gets stuck in terrain, buildings cannot be entered,
and property-based systems such as building item use and maglock passkeys
cannot be tested through normal Qt gameplay.

**Current Stoker QA result:** the physical arrival route is resolved in the
conversion. Scene-2 record 91 (`0x173138`, value `0xC2`) at the visible motel
doorway is reached with normal Up input. The route publishes raw collision
`0xFFC2`, event 2, and field mode 2; ROM `0x1F65A` then selects the Stoker
arrival record. Arrival, manager choice, Leave notice, and clean field return
are covered without injecting a mode/event tuple. The distinct scene-2 record
2 candidate at `0x172DBE` remains unrelated and still produces event 0; see
[`visual-audits/stoker-collision-candidate.md`](visual-audits/stoker-collision-candidate.md).
The other terrain, building, maglock, and item-use routes remain open.

**Solve:** compare movement probes, collision margins, terrain gates, and
property/event publication with ROM movement callbacks at `0xB7B8` and related
tables. Trace the actual building and Stoker collision records, then connect
the discovered event tuple to the existing typed dialog/property handlers.
Keep the dialog window generic; speaker properties and the triggered event
choose its text and choices. Route shops, maglock/passkey checks, and item use
from ordinary Qt controls only where the ROM exposes those actions.

**Done when:** each route is reachable from gameplay with normal input; a
source-backed typed state transition, Debug/Release CTest, and full-screen
entry/result/return captures exist; adjacent terrain and HUD behavior also
survive; Stoker's dialog is reached by the physical source property without
an injected event tuple.

**Status:** partial. `STOKER-01` is resolved for the physical motel doorway;
general terrain recovery and the other building/property routes remain open.

### STOKER-01 — Physical Stoker property and collision-to-dialogue route

The negative-candidate QA is complete in commit `c411247cb5`: scene-2 terrain
record 2 at `0x172DBE` is unrelated to the motel entrance and produces event 0
without Stoker dialogue. The positive physical route is now covered by
`shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`.
New Game begins in scene 2 with FFDF40 clear at `(0x3CE0,0x2D00)`. Ordinary Up
reaches terrain record 91 at `0x173138` (`0xC2`, bounds `0x3C0..0x3DF` by
`0x280..0x29F`), publishes raw collision `0xFFC2` and event 2, and enters the
mode-2/event-2 Stoker prompt at `0xF4AA0`. Ordinary A/B input advances through
the manager and Leave pages; A returns to a clean scene-2 field. The QA uses a
fixed source-PRNG seed only to make the ordinary arrival command choose the
manager branch repeatably. It does not inject collision, mode, event, or prompt
state.

Independent section 03 frames `000028..000054` show the runner at the motel
doorway, `000055` shows the dark transition, `000080..000081` show the blank
shared panel, and `000082` shows the Stoker arrival text and portrait. These
frames support the screen sequence and panel appearance, but contain no input
or event log. The native sequence currently goes from full black to the
populated panel; the exact duration represented by the two sampled blank
frames is not established by the ROM timing trace and remains an open visual
timing check.

**Done when:** ordinary field input contacts the source-confirmed property;
typed game state publishes the ROM-confirmed speaker/event state; the generic
dialog selects Stoker content from that state; a Debug/Release CTest and
whole-screen entry/dialogue/return dumps pass; no injected event tuple is
used.

**Status:** resolved for the scene-2 motel arrival route. The ROM collision,
normal-input publication, event-selected Stoker content, full-screen draw and
clean return are defended in Debug and Release. Independent footage confirms
the visible doorway/fade/dialogue order but not the controller bytes; the
two-frame blank-panel timing and dialogue audio remain unverified.

### HUD-01 — Runner portrait palette

**Reported:** field portrait color looks wrong.

**Source trace:** At `0x3560`, the face stamp is 4x4 at F1C6 and uses live
record `+0x2C` (`0` falls back to `0x2000`). The item stamp is 3x2 at F448 and
uses `0x3602`, whose branch reads `+0x57/+0x56/+0x9B`. The 13 selectable face
records use palette lines 1, 2, or 3.

**Code changed:** typed Qt state carries the raw `+0x2C` word; offline assets
include all three named portrait palettes; Qt selects item and unarmed palettes
with `0x3602`; the static panel plan uses the ROM's 4x4 face and 3x2 item sizes.

**Internally checked:** the zero-word fallback and all three face palette
indexes have regression coverage. Debug and Release graphics, typed HUD, field
action, and Qt QA pass. The Gator QA compares the rendered face against the
exact asset selected from its live record. All 12 full-screen HUD captures and
four full-screen player-action captures were reviewed; Debug/Release hashes
match.

**Independently visually checked:** section 04 frames 000900/000906/000912 and
section 06 frames 000001/000015/000030 show the same Gator face palette. The
full 320x224 frames were reviewed. Against the opaque 32x32 crop, line 1 has
17.713 mean absolute RGB error, versus 34.191 for line 2 and 37.849 for line 3;
the remaining difference is consistent with video compression.

The full eight-question review and Debug/Release dump list are in
[`visual-audits/hud-portrait-palette.md`](visual-audits/hud-portrait-palette.md).

**Status:** resolved for the reported Gator face-color symptom. Recruited-party
portrait fidelity remains unverified because no recruited HUD appears in the
supplied recording.

### HUD-02 — Ammunition state across equip and reload

**Reported:** the user observed 32 bullets initially, 37 while the gun was
unequipped, and 4 after equipping it.

**Source trace:** ROM field refresh `0x39F2` calls `0x38FA`, which iterates the
three runner records and calls `0x36FC` with `D1=0`. For clip-using item codes
below `0x0D`, `0x36FC` caps `+0x4E` to that item's
`D4540 + (item_code - 1) * 0x30 + 0x24` capacity before drawing the counter.
The Gator archetype template at `0x72B1A` has
item code 2 and raw `+0x4E=32`; item row 2 at `0xD4570` has capacity 12. The
ROM refresh therefore displays 12, not the unrefreshed template value 32.
`0x125FA` converts an equipped magazine to a percentage on unequip, and
`0x121E0` scales that percentage to the next weapon's capacity. For item code
3, ROM capacity is 11 (`0xD45A0 + 0x24`). Four of eleven rounds
become 37% on unequip under the source ceiling rule; 37% scales back to four
rounds when that item is equipped again. A 12-round weapon also maps 37% to
four by the source half-up scale. This establishes concrete source arithmetic,
not the user's exact weapon/state. The independent recording has no matching
equip/unequip sequence. Reserve `+0x4F` is tracked separately by `LOADOUT-01`.

**Code changed:** `FieldSceneRuntime::refresh_field_hud_runner_slot()` now
applies the typed zero-delta ammo-meter refresh to each live record before
building its HUD slot snapshot. It writes the source-clamped ammo and any
source item-clear fields back to typed runner state.

**Internally checked:** a typed regression asserts item code 2, 32 rounds,
and a zero-delta refresh produce 12 and the correct counter tiles. The
inventory-action regression checks code 3 capacity 11 and the exact
4/11 -> 37 -> 4 equip/unequip round trip. The direct ROM-backed graphics test
preserves Gator's raw template value 32 and asserts the item-3 capacity 11.
Field-HUD and field-action CTests pass 2/2 in Debug and Release; graphics and
catalog CTests pass 2/2 in both configurations. All 28 current full-screen
field-action checkpoints were reviewed; the Debug and Release BMP hashes
match. The route covers inventory selection, equip, cancel, unequip, weapon
switching, reload confirmation, empty magazine, and return to the Clips parent.

**Independently visually checked:** section 06 frames `002550`, `002560`,
`002580`, `002610`, `002640`, and `002699`, plus section 07 frames `000030`
and `000120`, were reviewed as complete 320x224 images. They show neighboring
field HUD, inventory, Magic, and First Aid states, but none shows the reported
weapon transfer. Independent confirmation of the precise 32/37/4 sequence is
unavailable.

**Status:** partial. The host now follows the source's initial capacity
refresh, and ROM arithmetic explains a plausible percentage-to-rounds
conversion. Keep the combined reported sequence open until an independent
capture or a reproduced normal-play state identifies the weapon, capacity,
and live values.

### MENU-01 — Pocket Secretary navigation, Magic, and Cyberware pages (`SOUND-01/02`, `SEC-MAGIC`, `SEC-CYBERWARE`)

**Reported:** moving between options lacks its navigation sound; selection
and cancel sounds are missing on some pages; Magic leaves old menu text
visible over its own content; Cyberware hides inventory, portrait, posture,
and text.

**Solve:** trace the Pocket Secretary root selection events and each child
page's source draw/clear path. Bind move/select/cancel to the correct audio
requests. Model Magic and Cyberware as typed pages that redraw their own
panels and preserve only the source-shared HUD.

**Done when:** full-screen captures cover root, every relevant child,
selection, return, and reopen; no old text/buttons survive page replacement;
all visible areas match the relevant independent section 06 frames; audio
events, playback start, and durations are recorded per action.

**Status:** pending current-build reproduction.

### MENU-02 — Start-menu Attributes/Skills Karma selection (`KARMA-COLOR`, `KARMA-DETAIL`)

**Reported:** selection color is wrong and specific selected attributes/skills
lack their detail text.

**Solve:** keep the view-only Start-menu Karma page separate from the Hotel
Karma spending page. Trace selector phases and the lower detail text by row;
bind them to typed selection without opening the page merely on hover.

**Done when:** all selectable rows and return transitions have full-screen
dumps, source-derived selection colors/detail text, no Hotel spending controls,
and tests proving hover does not open the page or spend Karma. Use matching
independent video evidence if present; otherwise keep visual fidelity
unverified.

**Status:** pending current-build reproduction.

### MENU-03 — Clips child and posture selector (`CLIPS-REDRAW`, `POSTURE-FOCUS`)

**Reported:** Clips draws over its parent, and the posture bar has no selector.
Carried-forward requirement: posture is a menu reachable with Up/Down from
the menu buttons; its selection affects attack/defense and uses the proper
selector.

**Solve:** make Clips replace/restore the parent cleanly. Trace posture focus
entry, Up/Down navigation, selector animation/attributes, selected value, and
attack/defense consequences.

**Done when:** parent and child frames at entry, navigation, confirm/cancel,
and return show no doubled controls; tests check selector state and stat
effects; full-screen frames include the entire command panel and HUD.

**Status:** pending current-build reproduction.

### INVENTORY-01 — Move/trade item behavior

**Reported:** items cannot be moved or traded between inventory locations.

**Solve:** trace source cursor/cell state, source/destination validation,
confirmation/cancel, equipment restrictions, and resulting item records. Keep
the inventory as one authoritative typed state shared by Qt panels.

**Done when:** normal controls move valid items, reject invalid moves, and
preserve state on cancel; tests cover rearrangement and related equipment
icons/stats; full-screen captures verify source and destination cells plus
the surrounding interface.

**Status:** pending current-build reproduction.

### INVENTORY-02 — Item use and portrait/HUD redraw

**Reported:** using an item draws the portrait over other bars.

**Solve:** trace the item-use destination and full redraw order, including
portrait, bars, posture, message, and return to inventory. Clear the old page
before drawing the result where the ROM replaces it.

**Done when:** item-use action/result/return frames show each panel exactly
once with the active runner's state; CTest checks the redraw and state change;
full-screen images are reviewed across the complete route.

**Status:** pending current-build reproduction.

### EQUIP-01 — Item selector thickness; EQUIP-02 — Equip/unequip cell cleanup

For `EQUIP-01`, trace the selected inventory coordinates and the source
outline attributes separately from the equipped-item cell treatment. The
current Qt action frames and CTest show a one-pixel `#C4BE27` edge around the
24x16 item art; section 06 frames 002560/002580/002610/002640 show the same
slim marker. The earlier debug-only crop is enlarged 4x and is not a
source-size view.

For `EQUIP-02`, repeat equip, unequip, and replacement, and compare the whole
inventory grid with a freshly rendered baseline. Verify the icon, equipment
frame, selector, other panels, and return state together.

**Done when:** source selection and equipment state are separate typed state;
tests cover the selector perimeter and repeated equipment changes;
Debug/Release full-screen captures show no stale cell treatment after
unequip; and a matching independent transition is cited or explicitly
unavailable.

**Status:** `EQUIP-01` geometry is not reproduced as a current defect and is
supported by independent frames. `EQUIP-02`'s reported stale black box is not
reproduced in the current full-screen action route; exact independent
equip/unequip footage remains unavailable.

### LOADOUT-01 — Weapon magazine and shared reserve relationship

**Source finding:** equipped weapon capacity comes from its own item row at
`D4540 + (item_code - 1) * 0x30 + 0x24`. The runner's current loaded count is in
`+0x4E`; on unequip `0x125FA` stores its percentage in that same field, and
switching weapons scales the percentage to the selected item's capacity at
`0x121E0`. The reserve is a separate runner-wide Universal Gun Clips count at
`+0x4F`, consumed by the reload handler. The traced source does not keep a
separate spare-clip reserve for every weapon, so the native model must not
invent one.

**Current QA:** the field-action route switches between weapon rows with
capacities 6 and 12, carries 50% across the switch, and checks that reloads
consume the shared reserve. It also checks empty, reloaded, equipped, and
unequipped HUD states. The normal combat firing route and an independent
loaded-save transfer example remain open.

**Done when:** visible counts agree with source state in HUD, inventory, and
Clips; normal input can fire/reload; tests cover two capacities and reserve
updates; full-screen output is reviewed. A matching independent loaded-save
frame is cited or marked unavailable.

**Status:** source model clarified and internally exercised; partial until the
combat fire/reload route and any matching independent loaded-save evidence are
reviewed.

### PLAY-01 — Normal-control reachability for reported game systems

Provide end-to-end Qt input routes from field play to shops, item use inside a
building, and maglock passkey checks, including their source-backed entry,
result, and return transitions. Reuse the generic dialog view: speaker
properties and the triggered event determine the prompt and choices.

**Done when:** each route is reachable without injected event tuples; valid
and invalid choices update typed game state correctly; full-screen captures
show clean entry/result/return and adjacent field/HUD state; Debug/Release CTest
covers the full route.

**Status:** pending; depends on field collision and property/event discovery.

### AUDIO-01 — Source event placement and duration across the game

**Reported:** sound effects are missing or play at the wrong times, including
the SEGA shock, intro music, and menu navigation/selection/cancel actions.

**Solve:** audit each event in its owning goal from ROM callsite to audio
resource, trigger condition, playback, duration, looping, and stop. Do not
solve this by globally playing an event at screen entry.

**Done when:** every reported sound is tied to a source event and correct
state transition; player-state and available output-device evidence confirm
playback; measured duration and stop/loop behavior are recorded. Unknown
events remain explicitly open.

**Status:** pending; overlaps the owning screen goals by design.

## Dependencies and sequence

1. `STOKER-01` is resolved for the physical scene-2 motel doorway by ordinary
   input. `FIELD-02` remains open for general terrain recovery and other
   building/property routes; event injection does not close those routes.
2. Finish and verify `FIELD-01`'s source-backed movement family before using
   its screenshots as proof of field visual fidelity.
3. `INTRO-01/02/03` cover logo, title, timed opening, aircraft, and taxi
   defects; compare consecutive whole-screen frames before changing each
   layer or timeline.
4. `HUD-01`, `HUD-02`, `ITEMS-01`, and `LOADOUT-01` provide authoritative
   runner/item state to inventory, combat bars, and shop routes.
5. `MENU-01/02/03`, `EQUIP-01/02`, and `INVENTORY-01/02` are separate screen
   routes. Each must replace old controls cleanly and be validated in normal
   play as well as by its focused regression route.
6. `PLAY-01` depends on field collision/property discovery. Make each named
   shop, building-item, and maglock route reachable without event injection.
7. `AUDIO-01` and `SOUND-01/02/03` track source events across screens. Close
   each sound in the goal that owns its trigger; do not mark the audio group
   complete while any reported event remains unresolved.
