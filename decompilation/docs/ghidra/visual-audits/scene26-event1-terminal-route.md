# Scene-2 event 1 through Scene-26 resource activity and Security Terminal

Date: 2026-10-10

**Status:** partial. One live `FieldSceneRuntime` carries the ordinary New
Game route through scene-2 collision record 93, row 1 of the map event table,
scene-26 resource setup, the physical maglock/collision route, the D8 Security
Terminal result, and a physical Scene-26 event-31 main-menu/Exit route. The QA
seeds the passkey, Electronics skill, PRNG state, and adjacent-cell starting
position. It does not prove how story gameplay supplies those inputs or the
exact Genesis appearance.

## ROM route and native state

Scene-2 record 93 at ROM `0x17314C` is the physical `0xC1` collision. Normal
movement publishes event 1; row 1 of the active scene-2 event table at
`0x172CDC` selects resource scene `0x1A` (scene 26). The source dispatcher at
`0x167AE` prepares the resource map and its floor selection. In this fresh
route the extracted resource bound is 3 and the selected floor index is 0.

The same runtime instance enters scene 26 at `(0x0A00, 0x0500)`. Ordinary
controller input traverses its event-29 elevator selector, both event-2
maglocks, and the collision-defined corridor to records 124/125 at
`0x1D9E46`. Their D8 value produces event `0x18`, two negative collision
probes, and the `0x20EC2` Security Terminal continuation. Passkey item
`0x14`/quality 2 is added to the live Joshua record as a test fixture so the
route can cross both locks.

Ordinary A selects Electronics after the source ten-frame prompt delay. The
test sets active Electronics skill `+0x8A` to 10 and the source PRNG seed to
`0x1234`; successful access displays the first-floor text selected from the
entry's preserved `FFE12C` value. It requests sound event `0x30`. The result
stays static through its 30-frame wait; ordinary A returns to scene 26 with
the floor state intact. The full return image equals the current field-only
render.

After returning to Scene 26, the route places the runner in the open cell
immediately south of collision record 126 at `0x1D9E5A`. The record covers
`x/y=0x0200..0x021F` (grid cell 16/16), carries raw `0x00DF`, and takes the
direct event-31 continuation (`0x0DB26`). A normal Up input crosses into the
rectangle and opens the Lua-authored `Main Menu` with `Cyberspace` and `Exit`.
Down moves the Lua-owned cursor from `Cyberspace` to `Exit`; A restores the
captured Scene-2 map and actor state. The adjacent-cell seed is QA setup; the
trigger itself is crossed through the ordinary field input/movement path.
Cyberspace remains an explicit unconverted boundary.

## Eight-question full-screen review

1. **Does it look correct?** I reviewed all 674 full-screen BMPs in eleven
   chronological contact sheets, then checked the terminal and event-31 menu
   at native size. The route shows the expected map, HUD, terminal prompt and
   result, then a full-screen `Main Menu` with `Cyberspace` and `Exit`. The
   returned Scene-2 image still has the known runner/HUD overlap at the event-1
   doorway; that visual anomaly remains open.
2. **Does the look reflect the behavioral state?** The route trace records
   event 1 and scene `0x1A`, selected floor 0, both passkey door changes, D8
   event `0x18`, terminal result, return to Scene 26, physical event-31 entry,
   Lua cursor indices 0 then 1, and Exit restoring Scene 2. Passkey, skill,
   PRNG, and the adjacent-cell starting position are QA fixtures.
3. **Are the visuals independently correct?** No matching Genesis capture
   shows this event-1-to-terminal or event-31 menu route. The host uses `>` as
   a temporary cursor glyph; exact source selector tile art and Genesis pixel
   fidelity are not established. Internal assets and matching Debug/Release
   images are not an independent visual oracle.
4. **Is it animated, and when?** The field actor walks during held directional
   input. The terminal prompt uses its source ten-frame input delay and its
   result stays static through the 30-frame source wait. The event-31 menu is
   pixel-identical during 30 neutral frames. Its expected Genesis animation
   cadence is not independently established.
5. **Is there a sound, and how long?** Terminal success requests event `0x30`,
   mapped to persistent resource 048; audible playback and source stop
   lifetime remain unknown. The tested event-31 menu open, 30-frame idle, and
   cursor movement produced no new audio request. No separate cursor sound is
   identified in the traced helper.
6. **Are the other screen areas correct too?** Full frames include the map,
   runner, HUD, terminal dialog, menu, and return. The menu replaces the field
   composition; the Scene-2 actor/HUD overlap remains explicitly open.
7. **Do multiple frames support this interpretation?** The 674 captured
   frames include the Scene-2 prefix, 500 Scene-26 route updates, terminal
   waits, menu open/navigation/30-frame idle hold, and return. Debug and
   Release artifact sets match by relative path and SHA-256. This is one host
   route, not multiple independent Genesis examples.
8. **Are new controls drawn over stale controls?** The full-screen activity
   pane replaces the field HUD; no field buttons remain beneath it. The return
   restores the field-only composition with no leftover menu panel or choices.

## Verification and limits

`shadowrun_ghidra_conversion_qt_scene26_event1_terminal_qa` runs in Debug and
Release. Its output is
`decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/scene26-event1-terminal-qa/`;
`actual.bmp` is the first-floor result. The route keeps one native runtime
instance alive across both existing route paths, terminal action, and Exit.

This verifies the selected floor survives physical map traversal into the
generic terminal result and that Scene-26 record 126 opens a Lua-controlled
activity menu whose Exit returns to the captured source scene. The modal return
uses a 60-frame host fade and consumes the modal's held A input; that duration
is a presentation choice, not a ROM timing claim. Story-driven passkey
acquisition, non-fixture Electronics outcomes, Cyberspace, exact cursor art,
independent Genesis visuals, the scene-2 actor/HUD overlap, and event-`0x30`
playback/stop timing remain open.
