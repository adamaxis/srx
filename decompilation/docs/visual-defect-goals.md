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

| Goal | Reported issue, itemized | Evidence check and closure condition |
|---|---|---|
| `BOOT-01` | The shock figure has no sound when it shocks the SEGA logo, and the same effect plays at the wrong time when the Decker intro starts. | Trace both ROM sound requests and runtime triggers separately; capture the logo shock and Decker-scene entry; verify playback start, duration, and stop on the output device. |
| `BOOT-02` | The flame decal remains on the right after the logo is fully revealed. | Independent section 01 frame 000372 still shows the flame; frame 000384 and later settled-title frames do not. Match the source transition and verify clean removal over consecutive full frames. |
| `BOOT-03` | The animated microchip/circuit background does not move upward. | Independent section 01 frames 000390, 000396, 000420, and 000450 show changing background positions. Trace source offset, direction, cadence, and lifetime; compare a multi-frame full-screen dump. |
| `TITLE-01` | The title's “PRESS START” label does not blink in and out. | Independent section 01 frame 000900 shows it, 000906 hides it, and 000912 shows it again. Match the ROM phase/counter and verify both phases over multiple cycles. |
| `TITLE-02` | Pressing Start skips the transition and immediately opens New Game/Load Game. | Trace the title Start edge through the transition into Start Options; capture every transition phase and confirm inputs do not reach the destination early. The independent video has a cut at this boundary, so it does not establish transition timing. |
| `OPEN-01` | Opening music starts on the second screen instead of when the opening starts. | Trace each timed opening stage's sound command and playback state; compare section 02 audio/frame timing and record duration/loop/stop behavior. |
| `OPEN-02` | In the Decker route, the head floats instead of staying attached to the character. | Identify the exact scene, actor record, and layer ownership; compare several adjacent frames and full-screen native captures. The sampled section 02 frames do not yet identify this Decker-specific state. |
| `OPEN-03` | In the Decker route, the body is superimposed over another image. | Trace scene handoff and layer clearing independently from head placement; verify no old actor/background layer remains across the full frame. |
| `OPEN-04` | The aircraft's rear propeller does not move. | Trace its frame list, cadence, and active-scene conditions; compare a continuous independent sequence and multi-frame full-screen dump. |
| `OPEN-05` | The aircraft lands on the building incorrectly. | Trace landing position, orientation, and layer order through the landing transition; review approach, contact, and settled frames. |
| `TAXI-01` | The taxi-window callout is missing its top/bottom edges and appears as two parallel side lines instead of a closed white outline. | Trace the source frame/layout and redraw ownership; compare full taxi frames before and after the callout. |
| `HUD-01` | The field portrait has unexpected color over the runner's face. | Trace runner identity, palette source, and portrait tiles; compare the same archetype and state in independent section 04/06 frames. Current QA has not yet established a same-archetype visual match. |
| `HUD-02` | Ammunition is inconsistent: the report cites 32 bullets initially, 37 on the unequipped hand, and 4 after equipping the gun. | Trace weapon identity, magazine capacity, and shared reserve through equip/unequip/fire/reload. Reconcile all three observed values against live source fields; do not assume any reported count is canonical. |
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
| `STOKER-01` | The physical Stoker collision/property that should open his generic dialogue is still unidentified. | The tested New Game scene-2 record 2 at `0x172DBE` is rejected: contact produces event 0 and no Stoker dialogue. ROM `0x1F65A` maps mode 2/event 2 to the Stoker arrival prompt, but the physical property that supplies the prompt event remains untraced. Test it through normal contact. |
| `PLAY-01` | Shops, building item use, and maglock passkeys cannot be tested from normal Qt gameplay. | Provide source-backed, normal-control routes from field play to each named feature and back; test them end to end without injecting event tuples. |
| `SOUND-03` | More generally, sound effects across the game are reported to play at the wrong points. | Inventory the reported and currently connected ROM sound requests, map each to a state transition, and check trigger, playback, duration, loop, and stop. Close individual sounds in their owning goals. |

### Earlier reported items carried forward

| Goal | Reported issue | Closure condition |
|---|---|---|
| `EQUIP-01` | The yellow item selector was previously reported as a thick rectangular mess instead of the game's slim yellow outline. | Verify selection state attributes and inspect full-screen equipped/unequipped frames; compare multiple independent item-menu frames and confirm the outline clears on deselection. |
| `EQUIP-02` | A black box can remain around an item icon after equipping and unequipping it. | Exercise repeated equip/unequip and replacement; prove the icon cell restores its correct background and no stale border/graphic remains. |
| `LOADOUT-01` | Weapon clips/ammunition were previously reported as not being tied to the weapons. | Trace each weapon's loaded count/capacity and the shared reserve separately. Current Clips QA describes a shared Universal Gun Clips reserve and weapon capacity from the item's `+0x24`; verify this against source records and normal fire/reload/equip routes before resolving the complaint. |

### Evidence gathered for this pass

- Independent section 01 frames were inspected in sequence. They support the
  flame lifetime, a changing title circuit background, and visible/hidden
  Push Start phases. The title-to-options transition is cut out of the video.
- The circuit's upward movement is now represented by the typed title offset
  and checked at offsets 0, 1, 2, and 24 in whole-screen Qt captures. During
  the same full-frame comparison, native title output remained much brighter
  and denser in green than independent title frames 390, 396, 420, 450, and
  1500. A top-band pixel statistic is diagnostic only because the independent
  frames are compressed and their exact VBlank alignment is unknown.
- The native `title_900.bmp` capture still has an orange sprite fragment at
  the right edge that is absent from independent frame 000900. Its source
  lifetime is still open under `BOOT-02`.
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
- Stoker candidate QA is committed, but scene-2 terrain record 2 at
  `0x172DBE` is far east/south of the motel and correctly produces event 0 with
  no Stoker dialogue. Scene-2 terrain record 91 at `0x173138` (`0xC2`, bounds
  `0x3C0..0x3DF` by `0x280..0x29F`) bounds the visible motel doorway. Normal
  New Game Up input contacts that rectangle, publishes event 0, and loads scene
  `0x14`; dialogue remains closed through 30 neutral updates. Independent
  section 03 frames `000028..000055` show the doorway approach and fade, while
  `000080..000082` show the blank and populated Stoker panels. That clip does
  not record controller input or the event state. The property/state that
  selects mode 2/event 2 remains unresolved.
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

**Status:** partial. `BOOT-03`'s source-cadenced upward circuit scroll is
implemented and covered by the Debug/Release startup QA. `BOOT-01` and
`BOOT-02` remain open; the full title still has the separately tracked palette
finding below.

### INTRO-02 — Push Start and title-to-start transition (`TITLE-01`, `TITLE-02`)

**Reported:** Push Start does not blink, and Start skips the transition into
New Game/Load Game.

**Solve:** identify the source blink counter/visibility interval and the
Start-edge transition/fade or wipe, then bind New Game/Load Game input only
after the transition reaches its destination state.

**Done when:** dumps include visible/hidden Push Start phases, Start pressed
and released, every transition phase, and the clean destination menu. The
independent section 01 frames support the blink phases; the title-to-options
transition is not shown in the supplied video and stays source-verified only
unless another matching capture is found.

**Status:** pending.

### TITLE-03 — Circuit background color and texture (added by full-screen audit)

**Observed:** the full-screen Qt title shows substantially more bright green
circuit detail than the independent title frames. In the top 16 rows of the
Debug `actual.bmp`, about 20% of pixels have green values at least 32; the
same diagnostic on independent section 01 frame 000450 is about 2%. This is a
triage measure, not a color-correction target, because the reference is
compressed video and the two captures are not phase-aligned.

**Solve:** trace the ROM background palette and the seven light phases, inspect
the offline extracted assets, and compare full frames at matched scroll and
light states. Correct the native palette/resource mapping only when the ROM
and independent frames support the change.

**Done when:** consecutive independent title frames and whole-screen Qt dumps
show the same circuit geometry, brightness range, and phase behavior while
the logo, prompt, and copyright remain in the correct layers.

**Status:** pending ROM palette and matched-state audit.

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

**Current Stoker QA result:** New Game scene 2 record 2 (`0x172DBE`) is a
type-2 collision object. Normal D-pad contact at world `(0x554,0x348)` produces
scene 2/event 0/projected code 0 and no Stoker dialog. It is a rejected Stoker
trigger candidate, not an implementation of Stoker interaction. See
[`visual-audits/stoker-collision-candidate.md`](visual-audits/stoker-collision-candidate.md).
Separately, New Game Up input reaches scene `0x14` through scene-2 event-table
row 0, with dialogue closed in the current QA. Longplay section 03 local frames
`000030`, `000060`, and `000090` show a city approach, dark transition, and
Stoker's arrival prompt; they do not identify the exact collision or input.
ROM `0x1F65A` selects Stoker's arrival prompt for mode 2/event 2, but the
physical property/event producer remains unidentified.

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

**Status:** pending follow-up. The scene-2 candidate QA is complete; the
actual Stoker trigger and terrain/building routes remain open.

### STOKER-01 — Physical Stoker property and collision-to-dialogue route

The negative-candidate QA is complete in commit `c411247cb5`: scene-2 terrain
record 2 at `0x172DBE` is unrelated to the motel entrance and produces event 0
without Stoker dialogue. ROM record 91 at `0x173138` (`0xC2`, bounds
`0x3C0..0x3DF` by `0x280..0x29F`) matches the visible motel doorway. Normal
New Game Up input contacts its collision margin, publishes event 0, and loads
scene `0x14`; the current Qt route leaves dialogue closed through 30 neutral
updates. The independent section 03 sequence shows the runner approaching that
door in frames `000028..000054`, a fade in `000055`, a blank generic panel in
`000080`, and the populated Stoker panel in `000082`. It does not log input or
the selected event. ROM `0x1F65A` maps mode 2/event 2 to the Stoker arrival
record at `0xF4AA0`; the state/property that publishes that tuple remains
untraced. Follow the physical contact, any scene-entry or action gate, generic
speaker/event selection, and return path before extending runtime behavior.

**Done when:** ordinary field input contacts the source-confirmed property;
typed game state publishes the ROM-confirmed speaker/event state; the generic
dialog selects Stoker content from that state; a Debug/Release CTest and
whole-screen entry/dialogue/return dumps pass; no injected event tuple is
used.

**Status:** open. The negative candidate test is complete; the physical trigger
is unresolved.

### HUD-01 — Runner portrait palette, ammo, equipment, and combat values

**Reported:** field portrait colors look wrong; the gun shows inconsistent
ammunition (the report cites 32 at one point, 37 while unequipped, and 4 while
equipped); armor, weapons, and their damage values are wrong.

**Solve:** trace the active runner record, portrait palette, magazine/reserve
fields, item identity, equipment changes, and stat/damage calculations. Treat
32/37/4 as observed values to reconcile, not as assumed canonical values.
Review equipped and unequipped states and transitions over the full HUD and
inventory.

**Done when:** the visible portrait palette and every count/stat agree with
source record state through equip, unequip, fire/reload, and runner switching;
tests cover data and full-screen output; independent section 04/06 frames are
used where they show the same state. Any state absent from the recording stays
labeled internally verified only.

**Status:** pending evidence reconciliation.

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

### EQUIP-01 and EQUIP-02 — Item selector and equip/unequip cleanup

Trace the source selection attributes and background ownership for each item
cell. Verify the slim selector across selected/unselected states, then repeat
equip, unequip, and replacement while reviewing every full-screen frame for a
black cell box or stale icon pixels.

**Done when:** item state and selector are bound to typed inventory state;
tests cover selection and repeated equipment changes; Debug/Release full-screen
captures show the same clean cell after each return; a matching independent
frame is cited or its absence is explicit.

**Status:** pending current-build reproduction.

### LOADOUT-01 — Weapon magazine and shared reserve relationship

Resolve the reported weapon-specific clip expectation from the ROM. Current
Clips QA says each weapon's capacity comes from its item record `+0x24` while
the Universal Gun Clips reserve is shared in runner record `+0x4F`. Follow the
loaded count, reserve, equip, fire, and reload across at least two weapon
types; do not add a per-weapon reserve unless the source supports it.

**Done when:** visible counts agree with source state in HUD, inventory, and
Clips; normal input can fire/reload; tests cover two capacities and reserve
updates; full-screen output is reviewed. A matching independent loaded-save
frame is cited or marked unavailable.

**Status:** source model needs comparison to the user's reported counts.

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

1. The scene-2 candidate collision QA is committed. `STOKER-01` and `FIELD-02`
   remain open until the real building/property producer is identified and
   tested with ordinary input; injected event tuples do not close them.
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
