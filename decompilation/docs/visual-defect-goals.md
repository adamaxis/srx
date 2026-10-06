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

## Goals

### INTRO-01 — SEGA shock and logo reveal

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

**Status:** pending source/evidence audit.

### INTRO-02 — Push Start and title-to-start transition

**Reported:** Push Start does not blink, and Start skips the transition into
New Game/Load Game.

**Solve:** identify the source blink counter/visibility interval and the
Start-edge transition/fade or wipe, then bind New Game/Load Game input only
after the transition reaches its destination state.

**Done when:** dumps include visible/hidden Push Start phases, Start pressed
and released, every transition phase, and the clean destination menu; the
independent section 01 frames support the sequence and timing within the
30-fps reference's limits.

**Status:** pending.

### INTRO-03 — Decker, aircraft, landing, and taxi frame

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
independent section 02 frames support placement/order, and music timing is
verified from the source request through playback.

**Status:** pending.

### FIELD-01 — Field actor movement pose and animation

**Reported:** held movement appears to use a dying/fallen pose.

**Current evidence:** the Qt field movement presenter selects its extracted
`0xC0B2` family on every accepted movement. ROM `0xB330` and the no-`+0x4C`
branch of `0xAFF2` select `0xC0B2` during behavior transitions; this alone
does not prove that ordinary held-D-pad movement uses that family. The
independent section 04 frames `000900..000911` show successive field movement
states, but the input/record selector is not captured. Resolve the runtime
selector against the actual movement and idle source paths before changing
the sprite family or declaring the appearance correct.

**Solve:** trace the active runner's selector, behavior flags, direction,
frame index, and reset on D-pad hold/release. Compare candidate ROM families
against several independent movement frames. Only then change the typed
animation state/assets and add a regression that fails if normal movement
selects a hurt/death/transition pose. Include idle, heading changes, blocked
movement, and return-to-idle.

**Done when:** ROM selector and cadence are documented; the Qt runtime consumes
the corresponding named assets directly; the regression verifies selection
and timing across multiple headings and release; Debug/Release full-screen
dumps pass review; matching independent frames support the pose, or the lack
of a precise comparison is stated. The separate collision/building goal does
not get folded into this visual claim.

**Status:** active investigation. No current-build defect fix is claimed.

### FIELD-02 — Terrain collision, building access, and physical Stoker trigger

**Reported:** movement gets stuck in terrain, buildings cannot be entered,
and property-based systems such as building item use and maglock passkeys
cannot be tested through normal Qt gameplay.

**Current Stoker QA result:** New Game scene 2 record 2 (`0x172DBE`) is a
type-2 collision object. Normal D-pad contact at world `(0x554,0x348)` produces
scene 2/event 0/projected code 0 and no Stoker dialog. It is a rejected Stoker
trigger candidate, not an implementation of Stoker interaction. See
[`visual-audits/stoker-collision-candidate.md`](visual-audits/stoker-collision-candidate.md).
The physical property that publishes mode 2/event 2 remains unidentified.

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

### MENU-01 — Pocket Secretary navigation, Magic, and Cyberware pages

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

### MENU-02 — Start-menu Attributes/Skills Karma selection

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

### MENU-03 — Clips child, posture selector, and clean page replacement

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

1. `FIELD-01` is the current active audit: resolve the movement pose and
   ordinary D-pad animation selector before changing it.
2. `FIELD-02` follows with the physical building/property route; the
   scene-2 record-2 candidate is already rejected.
3. `INTRO-01` and `INTRO-02` cover boot/title timing before the Decker/taxi
   continuation in `INTRO-03`.
4. `HUD-01` supplies authoritative runner/equipment state to
   `INVENTORY-01/02`, `MENU-03`, and building item-use routes in `FIELD-02`.
5. `MENU-01/02/03` and inventory goals are separate screen hills; each must
   replace old controls cleanly and be validated as its own route.
6. `AUDIO-01` is shared tracking, but each sound is closed by the screen goal
   that owns its trigger. Do not mark the umbrella complete while any reported
   event remains unresolved.
