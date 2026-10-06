# Stoker Motel Doorway Collision-to-Dialogue Audit

Date: 2026-10-06

Primary source trace: US ROM New Game field setup and coordinator (`0x075E`,
`0xB7B8`, `0x16396`, `0x1F5CA`, `0x1F65A`, `0x1F246`, `0x1F3C4`, `0x2BD2`),
scene-2 terrain record 91 (`0x173138`), Stoker prompt records
(`0xF4AA0..0xF4B2C`), and profile/portrait records (`0x121FA0`, `0x122420`).
Ghidra output was used only as a cross-check. Runtime code is the typed Qt field
controller and shared dialogue panel; it does not execute ROM code or inject
the collision/event tuple.

CTest: `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`.
Output: `qt_runtime/field-stoker-collision-to-dialogue-qa/` in Debug and
Release builds.

## Source route and runtime result

- New Game starts in scene 2 at fixed-point position `(0x3CE0, 0x2D00)` with
  `FFDF40` clear (`world_mode = 0`). The scene descriptor's terrain-collision
  record 91 at `0x173138` has bounds `(0x3C0..0x3DF, 0x280..0x29F)` and value
  `0xC2`. This is the physical motel doorway seen in section 03.
- Ordinary Up input approaches the rectangle. The movement collision margin
  publishes raw `FFDF64 = 0xFFC2`; the low five bits become event 2. On the
  collision frame the field activity route is mode 2/event 2. The test records
  contact at `(0x3CE, 0x2A5)`, within the source rectangle's expanded margin.
- `0x16396` advances the fade. Collision-frame state is timer 15 and one
  palette step. The source timer takes 17 updates to reach full black. The
  coordinator then runs `0x1F5CA`; `0x1F65A` recognizes mode 2/event 2 and
  selects the Stoker arrival record at `0xF4AA0`. The value `0x14` in this
  selector is an exit marker; it is not a destination scene ID.
- The shared `0x1F246`/`0x1F3C4` dialogue route presents Stoker's direct 56x72
  portrait and text catalog record. A on `< MORE >` executes the ordinary
  arrival command `0xE9`. Its source random sample 0..79 chooses the manager
  greeting; the QA fixes the source PRNG seed only to make that normal branch
  repeatable. B chooses the second visible manager option and opens the Leave
  notice. A dismisses it, performs same-scene `0x2BD2` re-entry, refreshes the
  native field actor projection, and returns to scene 2 with no old dialogue
  pixels or pending transition request.
- Scene-2 record 2 at `0x172DBE` is a separate, rejected candidate. It remains
  far east/south of the motel and its normal contact produces event 0. It is
  not part of the positive Stoker route; see
  [`stoker-collision-candidate.md`](stoker-collision-candidate.md).

## Full-screen and independent frame review

The QA writes exactly twelve full 320x224 BMPs: `spawn`, `movement-8`,
`movement-16`, `doorway-trigger`, `fade-08`, `fade-16`, `fade-17`,
`stoker-arrival-dialogue`, `stoker-manager-greeting`, `stoker-leave-notice`,
`returned-to-field`, and `actual`. It also writes `route.tsv` and
`source-boundaries.txt`. Stale BMPs from earlier probes are removed before the
exact output-set check. Debug and Release outputs are compared by file name and
SHA-256 after both runs.

Every native frame was opened at its full 320x224 size. The two movement
checkpoints show Up input moving the actor toward the door. The doorway frame
shows a blocked contact without a new modal. Fade checkpoints 08, 16, and 17
show a progressive darkening, full black, and the completed fade. The next
frame is the shared Stoker panel. Its portrait and green text are visible, the
field HUD is absent, and `< MORE >` appears in its lower region. The manager
frame replaces the first page with two A/B options; the Leave frame removes
those options and displays its text. The returned frame restores the map,
runner, and right HUD with no leftover dialogue shell or text. No second set of
controls is drawn over the old screen.

Independent source visuals were reviewed from the supplied longplay at full
320x224 size: section 03 frames `000028`, `000035`, `000042`, `000048`, and
`000054` show the approach; `000055` shows the dark transition; `000080` and
`000081` show the blank shared panel; `000082` shows the populated Stoker
portrait/text/`< MORE >` panel. The native panel's dimensions, text placement,
portrait, and continuation region match that screen family. The recording has
no controller or collision log, so it supports appearance and sequence, not the
meaning of the input or event byte. It samples a blank panel twice before
text appears, whereas the current Qt route moves from full black directly to
the populated page. The ROM trace does not yet establish the duration of that
sampled blank interval; keep the exact transition timing open. The field
portrait palette and ammunition visible in the surrounding native HUD remain
tracked under `HUD-01`; this Stoker audit does not claim those unrelated values
or palette are correct. The video route's input/archetype state is also not
logged, so movement timing and portrait-color comparisons are not treated as
same-state matches.

## Eight-question visual review

1. **Does it look correct?** The door approach, dark transition, and generic
   Stoker panel agree with independent section 03 frames. Exact player-input
   timing cannot be compared because the clip does not log the controller. Qt
   currently skips the sampled two-frame blank-shell interval; its ROM-derived
   duration is unresolved.
2. **Does the look reflect behavior?** Yes for the tested route. The actor
   reaches record 91, raw collision `0xFFC2` publishes event 2, and the
   mode-2/event-2 selector opens the Stoker arrival content. Controller A/B
   advances through the source manager and Leave records. Return clears the
   panel and transition request.
3. **Are the visuals correct?** The doorway, panel frame, portrait, green text,
   and continuation region match the independent screen family at the reviewed
   size. The exact transition timing and same-archetype HUD portrait palette
   are not resolved here; the latter remains `HUD-01`.
4. **Is it animated?** The controlled actor changes pose and position in the
   two normal-input movement checkpoints. The Stoker speaker uses a single
   56x72 portrait resource, not an identified portrait animation list. The
   dialogue remains static until a new controller edge changes the record.
   Fade cadence follows `0x16396`: 17 updates with palette steps reaching 9.
   The independent clip supports approach/fade/panel order but not exact
   VBlank alignment.
5. **Is there sound?** No direct collision, fade, or dialogue SFX request was
   identified in this ROM route. The coordinator does run mode-entry audio
   service `0x53126`: six command-`0x14` transactions, one three-VBlank wait
   group per transaction (18 VBlanks total), a flush, and one service tick. The
   Qt backend records those boundaries and does not play them. The route logs
   zero discrete SFX callbacks. No sound alias or audible duration for the
   mode-entry command was verified, so this is not evidence that the original
   game is silent.
6. **Are the rest of the screen and related elements correct?** The full
   captures include the entire map, actor, right HUD, panel, and return frame.
   Dialogue replaces the field contents cleanly; no old buttons, HUD, map, or
   portrait is left under the new panel. The visible field portrait/ammo state
   remains outside this goal and is tracked separately.
7. **Do multiple frames support the interpretation?** Yes. Native entry,
   movement, collision, three fade phases, three dialogue states, and return
   frames are checked. Independent frames cover five approach samples, the
   fade, two blank-panel samples, and the populated Stoker panel. The clip
   lacks input/event evidence, so behavior is source- and native-state-backed.
8. **Was the previous UI hidden or redrawn before the new one?** Yes. The
   shared full-screen panel is drawn without the field HUD; after Leave, the
   field screen is re-entered and redrawn before the QA accepts the return.
   Whole-frame review shows no buttons or graphics from the previous screen
   underneath either state.

## Outcome

- **Code changed:** New Game preserves source startup `world_mode = 0`; movement
  state distinguishes quantizer return from raw `FFDF64`; Stoker prompt
  selection clears the completed fade overlay; the coordinator now suspends
  scene re-entry while dialogue owns the screen and refreshes the actor
  projection on return.
- **Behavior implemented:** normal Up reaches the ROM C2 doorway, publishes
  event 2, and opens the generic Stoker arrival record. Ordinary A/B/A input
  follows arrival, manager Leave, and clean return. No event tuple or prompt is
  injected.
- **Internally checked:** Debug/Release route CTests assert collision bounds,
  raw and projected codes, mode/event state, fade counters, the typed speaker,
  portrait alias and text, A/B choices, transition suspension, audio service
  transaction counts, and whole-screen return. The complete Qt CTest suite
  passes 53/53 in Debug and Release. All twelve full-screen output names and
  hashes match across configurations; every Debug image was reviewed at full
  size.
- **Independently visually checked:** section 03 frames `000028..000055` and
  `000080..000082` support the door/fade/panel appearance and order. They do
  not reveal controller input, collision data, or the source event. The sampled
  blank-shell timing remains open.
- **Reported defect resolved:** the physical scene-2 Stoker collision and
  normal-input dialogue route are resolved in the Qt conversion. General
  building access, same-archetype HUD palette/ammo, exact blank-shell timing,
  and actual audio playback are not closed by this hill.
