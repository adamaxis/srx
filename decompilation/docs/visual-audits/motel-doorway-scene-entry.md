# Stoker Motel Opening Auto-Walk and Doorway Collision Audit

Date: 2026-10-08

Primary source trace: US ROM opening marker (`0x59C5E`), New Game field loop
and entity action/movement path (`0x075E`, `0xACAE`, `0xB574`, `0xBADE`,
`0xB7B8`, `0x16396`, `0x1F5CA`, `0x1F65A`, `0x1F246`, `0x1F3C4`, `0x2BD2`),
scene-2 terrain record 91 (`0x173138`), Stoker prompt records
(`0xF4AA0..0xF4B2C`), profile/portrait records (`0x121FA0`, `0x122420`),
and the shared `0x1728` sprite-stream writer.
Ghidra output was used only as a cross-check. Runtime code represents the
startup action as typed field state and uses the ordinary native movement,
collision, event, and shared dialogue systems; it does not execute ROM code or
inject the collision/event tuple.

CTest: `shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa`.
Output: `qt_runtime/field-stoker-collision-to-dialogue-qa/` in Debug and
Release builds.

## Source route and runtime result

- `0x59C5E` sets `FFE18B` during the New Game opening. `0x075E` then loads the
  actual scene-2 field map and starts Joshua at `(0x3CE0, 0x2D00)` with
  `FFDF40` clear (`world_mode = 0`). After 31 entity-service warmup passes,
  `0xACAE` sees `FFE18B` still set. `FFE18C` is zero, so it sets the actor's
  target heading to zero and installs the ordinary movement continuation
  `0xB574`.
- Heading-zero movement runs through the standard walk animation, `0xBADE`
  step, terrain probes, and `0xB7B8` collision dispatcher. It reaches record
  91 at `0x173138`, whose bounds are `(0x3C0..0x3DF, 0x280..0x29F)` and value
  `0xC2`. No controller input drives this approach. The movement collision
  margin publishes raw `FFDF64 = 0xFFC2`; the low five bits become event 2.
  The route is mode 2/event 2 and the QA records contact at `(0x3CE, 0x2A5)`,
  within the source rectangle's expanded margin.
- `0x16396` advances the fade. Collision-frame state is timer 15 and one
  palette step. The source timer takes 17 updates to reach full black. The
  coordinator then runs `0x1F5CA`; `0x1F65A` recognizes mode 2/event 2 and
  selects the Stoker arrival record at `0xF4AA0`. The value `0x14` in this
  selector is an exit marker; it is not a destination scene ID.
- After the first exit-orchestrator pass, `0x075E` clears `FFE18B`. This
  one-shot movement is complete; the Stoker interaction itself continues in
  scene 2. There is no separate motel interior or dedicated opening walk
  animation.
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

Every native frame was opened at its full 320x224 size. The movement
checkpoints show the automatic startup action moving the actor toward the
door while the controller nibble stays zero. The QA asserts that controller
input remains released on every movement update. The doorway frame
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

## Portrait extraction correction

ROM `0x1F292` loads Stoker's profile at `0x121FA0`, decodes pattern stream
`0x1287C6`, then sends the 7x9 tile stream at `0x122420` to `0x1728`. The
stepped-descriptor branch ends each run with `BGE`, so its count is the low
five bits plus one. The same-delta branch ends with `BGT`, so it emits one
tile when the encoded count is zero or one, and otherwise emits the encoded
count. The previous importer used the stepped count for both branches. The
first same-delta descriptor (`0xE1`) therefore emitted an extra tile and
shifted later portrait tiles into the wrong cells.

The offline importer now follows those two distinct terminal conditions. The
`shadowrun_graphics` CTest checks that the tile after descriptor `0xE1` is not
a duplicate of its one-tile run. The named Stoker portrait was regenerated
with the explicit `shadowrun_graphics_extract --all-assets` step. A full Qt
`manager-package` dump is at
`build/conversion/qt/release/qt_runtime/field-dialogue-qa/manager-package/actual.bmp`.
At its 56x72 portrait crop, it matches independent longplay frames 000082,
000301, and 000601 at the same `(16,32)` placement with mean absolute RGB
error 8.91, 8.57, and 8.56 per channel. Those frames show the opening text,
manager options, and debt question; all include the same corrected Stoker
portrait. The screenshot remains video-derived, so the small per-pixel color
differences include capture compression.

## Nine-question visual review

1. **Does it look correct?** The automatic door approach, dark transition,
   generic Stoker panel, and corrected portrait agree with independent section
   03 frames. The clip has no controller log; the no-input interpretation
   comes from the ROM trace and native runtime state checks. Qt currently skips
   the sampled two-frame blank-shell interval; its ROM-derived duration is
   unresolved.
2. **Does the look reflect behavior?** Yes for the tested route. The completed
   New Game opening arms heading-zero movement while the controller stays
   released. The actor reaches record 91, raw collision `0xFFC2` publishes
   event 2, and the mode-2/event-2 selector opens the Stoker arrival content.
   Controller A/B advances through the source manager and Leave records.
   Return clears the panel and transition request.
3. **Are the visuals correct?** The doorway, panel frame, ROM-corrected Stoker
   portrait tile layout, green text, and continuation region match the
   independent screen family at the reviewed size. The exact transition timing
   and same-archetype HUD portrait palette are not resolved here; the latter
   remains `HUD-01`.
4. **Is it animated?** The actor changes position and uses the ordinary field
   walk poses during the automatic movement. There is no separate opening
   movement animation. The Stoker speaker uses a single 56x72 portrait
   resource, not an identified portrait animation list. The dialogue remains
   static until a new controller edge changes the record.
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
   automatic movement, collision, three fade phases, three dialogue states,
   and return frames are checked. Independent frames cover five approach
   samples, the fade, two blank-panel samples, and Stoker's opening, manager,
   and debt pages (000082, 000301, and 000601). The clip lacks input/event
   evidence, so the behavior claim is ROM- and native-state-backed.
8. **Was the previous UI hidden or redrawn before the new one?** Yes. The
   shared full-screen panel is drawn without the field HUD; after Leave, the
   field screen is re-entered and redrawn before the QA accepts the return.
   Whole-frame review shows no buttons or graphics from the previous screen
   underneath either state.
9. **Was the whole available sequence reviewed?** Yes. All twelve native
   full-screen captures were checked, including the two movement frames,
   collision, fade, dialogue replacement, and clean field return. The
   independent video samples were inspected at full resolution; it supports
   screen appearance and sequence, not input state or exact frame cadence.

## Outcome

- **Code changed:** the startup coordinator carries completed New Game opening
  state into the field runtime; typed field state arms heading-zero automatic
  movement for that route only, runs it through existing movement/collision,
  and consumes it after the first field-exit orchestrator pass. Loaded saves
  and ordinary field QA bootstraps do not arm it.
- **Behavior implemented:** New Game reaches the ROM C2 doorway with zero
  controller input, publishes event 2, and opens the generic Stoker arrival
  record. Ordinary A/B/A input follows arrival, manager Leave, and clean
  return. No event tuple or prompt is injected.
- **Internally checked:** the startup integration and Stoker route CTests both
  pass in Debug and Release. They assert the completed-opening handoff,
  zero-input movement, collision bounds, raw and projected codes, mode/event
  state, fade counters, the typed speaker, portrait alias and text, A/B
  choices, transition suspension, audio-service transaction counts, and
  whole-screen return. All 14 output files (12 BMPs, `route.tsv`, and
  `source-boundaries.txt`) have matching Debug/Release SHA-256 hashes. Every
  Release BMP was opened and reviewed at full size; identical Debug hashes
  confirm those pixels match.
- **Independently visually checked:** section 03 frames `000028..000055` and
  `000080..000082` support the door/fade/panel appearance and order. They do
  not reveal controller input, collision data, or the source event. The sampled
  blank-shell timing remains open.
- **Reported defect resolved:** the physical scene-2 Stoker collision and
  automatic New Game arrival route are connected in the Qt conversion. General
  building access, same-archetype HUD palette/ammo, exact blank-shell timing,
  and actual audio playback are not closed by this hill.
