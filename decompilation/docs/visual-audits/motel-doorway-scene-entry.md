# Stoker Motel Opening Auto-Walk and Doorway Collision Audit

Date: 2026-10-08

Primary source trace: US ROM opening marker (`0x59C5E`), New Game field loop
and entity action/movement path (`0x075E`, `0xACAE`, `0xB574`, `0xBADE`,
`0xB7B8`, `0x16396`, indexed title-service dispatch (`0x53A78`), Stoker
service entry (`0x53AB6`), `0x1F5CA`, `0x1F65A`, `0x1F246`, `0x1F3C4`,
`0x2BD2`),
scene-2 terrain record 91 (`0x173138`), Stoker prompt records
(`0xF4AA0..0xF4B2C`), profile/portrait records (`0x121FA0`, `0x122420`),
and the shared `0x1728` sprite-stream writer.
Ghidra output was used only as a cross-check. Runtime code represents the
scene-entry condition in Lua and starts its handler as a yieldable task. The
handler requests heading-zero player movement through `game.actor`; the native
field controller executes the movement and resumes the script when the
one-shot action is consumed. Runtime code does not execute ROM code or inject
the collision/event tuple.

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
- The indexed service loop at `0x53A78` runs before the generic mode/event
  selector. Its first Stoker service is selected for scene 2/event 2 while
  FFFC1E bit 0 and FFF0D5 are clear. The Lua runtime loads story flags into
  global `game.state`, lets the quest script update them, and syncs them back
  after the call. The first visit opens the special sign/arrival page,
  and a later unpaid visit resolves to the three-choice manager page. A on the
  first page opens that offer, including the option to collect Michael's
  belongings; C opens the debt question and B opens the Jump House referral.
  On the paid route, command `0x05` checks the 250-nuyen threshold, applies
  the source wallet/story/runner mutations, and opens the package handoff
  record beginning at `0xF487C`. The live Qt route uses a documented 250-nuyen
  QA wallet seed; it checks the net +250 balance, handoff text, paid/collected
  story state, and clean field return. Lua story-resolution tests cover an
  unpaid repeat offer and the ordinary motel result after both completion
  flags are set. Restored-save wallet and story flags remain unresolved.
- Shared `0x1F246`/`0x1F3C4` presents Stoker's 56x72 portrait and text catalog
  record. Dismissing the paid handoff performs same-scene `0x2BD2` re-entry,
  redraws the native field actor projection, and returns to scene 2 with no
  old dialogue pixels or pending transition request. A separate physical
  repeat-visit route is not claimed: exploratory steering found the narrow
  record-91 lane adjacent to solid `0x7F` cells, and the correct post-dialogue
  exit/re-entry path still needs a source-backed route.
- Scene-2 record 2 at `0x172DBE` is a separate, rejected candidate. It remains
  far east/south of the motel and its normal contact produces event 0. It is
  not part of the positive Stoker route; see
  [`stoker-collision-candidate.md`](stoker-collision-candidate.md).

## Full-screen and independent frame review

The QA writes sixteen full 320x224 BMPs: `spawn`, `movement-8`,
`movement-16`, `doorway-trigger`, `fade-08`, `fade-16`, `fade-17`,
`stoker-arrival-dialogue`, `stoker-manager-package-offer`,
`stoker-debt-question`, `stoker-package-handoff`, `returned-to-field`,
`stoker-manager-ordinary`, `stoker-leave-dialogue`,
`stoker-leave-returned-to-field`, and `actual`. It also writes `route.tsv` and
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
field HUD is absent, and the A-button marker appears beside `< MORE >`. The
manager frame replaces the first page with A/B/C choices, including Michael's
package offer. The debt question and paid package handoff replace those choices
with their own source text. The ordinary manager page has two rows; its Leave
branch replaces both rows with a title-only farewell page. The returned frame
restores the map, runner, and right HUD with no leftover dialogue shell or
text. No second set of controls is drawn over the old screen.

Independent source visuals were reviewed from the supplied longplay at full
320x224 size: section 03 frames `000028`, `000035`, `000042`, `000048`, and
`000054` show the approach; `000055` shows the dark transition; `000080` and
`000081` show the blank shared panel; `000082` shows the populated Stoker
portrait/text/`< MORE >` panel. The native panel's dimensions, text placement,
portrait, A-button marker, and continuation region match that screen family.
The recording has
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

The user flagged Stoker's picture as incorrect and suspects the shared head
extraction path. I re-opened the named 56x72 asset, the current Qt frames, and
independent frame 000301 at the same `(16,32)` crop; this recheck still shows
the same portrait shape and tile layout. That evidence does not validate the
separate runner HUD faces or other animated character-head assets, which need
their own ROM-first and multi-frame audit before being called correct.

## Nine-question visual review

1. **Does it look correct?** The automatic door approach, dark transition,
   corrected Stoker portrait, opening page, A-button marker on `< MORE >`, and
   three-choice manager offer match the relevant independent section 03
   captures. The clip has no controller log; the no-input interpretation comes
   from the ROM trace and native runtime state checks. Qt still skips the
   sampled two-frame blank-shell interval; its ROM-derived duration is
   unresolved.
2. **Does the look reflect behavior?** The completed New Game opening arms
   heading-zero movement while the controller stays released. Lua requests
   that movement with `game.actor.walk_heading("player", 0)`, yields, and
   resumes after the native field-exit coordinator consumes it. The actor
   reaches record 91, raw collision `0xFFC2` publishes event 2, and the event
   system resolves the Stoker script through indexed title service `0x53A78`.
   The first entry opens the special page; A opens the three-choice package
   offer, C opens the debt question, and B selects the Jump House referral.
   The live paid route uses a 250-nuyen QA wallet seed, runs command `0x05`,
   verifies the net +250 balance and source story/runner mutations, shows the
   package handoff page, and returns to the field. Typed state tests select
   the offer on an unpaid repeat and ordinary motel service only when both
   debt-paid and package-collected flags are set. Physical second-entry
   movement and loaded-save restoration remain open.
3. **Are the visuals correct?** The doorway, panel frame, ROM-corrected Stoker
   portrait tile layout, green text, A-button marker, three-choice labels, and
   continuation region match the independent screen family at the reviewed
   size. The exact transition timing
   and same-archetype HUD portrait palette are not resolved here; the latter
   remains `HUD-01`.
4. **Is it animated?** The actor changes position and uses the ordinary field
   walk poses during the automatic movement. There is no separate opening
   movement animation; Lua requests movement but does not directly set
   position or animation. The Stoker speaker uses a single 56x72 portrait
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
   zero discrete SFX callbacks. Lua audio playback is not connected yet. No
   sound alias or audible duration for the mode-entry command was verified,
   so this is not evidence that the original game is silent.
6. **Are the rest of the screen and related elements correct?** The full
   captures include the entire map, actor, right HUD, panel, and return frame.
   Dialogue replaces the field contents cleanly; no old buttons, HUD, map, or
   portrait is left under the new panel. The visible field portrait/ammo state
   remains outside this goal and is tracked separately.
7. **Do multiple frames support the interpretation?** Yes. Native entry,
   automatic movement, collision, three fade phases, opening, manager offer,
   debt question, paid handoff, and return frames are checked. Independent
   frames cover five approach samples, the fade, two blank-panel samples,
   Stoker's opening (000082), manager offer (000301), debt (000601), and
   referral (000690/000750/000780/000840). The paid handoff has ROM and
   runtime-state evidence but no matching longplay frame. The clip lacks
   input/event evidence, so the behavior claim is ROM- and native-state-backed.
8. **Was the previous UI hidden or redrawn before the new one?** Yes. The
   shared full-screen panel is drawn without the field HUD; after the paid
   handoff, the field screen is re-entered and redrawn before the QA accepts
   the return. Whole-frame review shows no buttons or graphics from the
   previous screen underneath either state.
9. **Was the whole available sequence reviewed?** Yes. All sixteen native
   full-screen captures were checked, including the two movement frames,
   collision, fade, arrival, manager, debt question, paid handoff, and clean
   field return. The independent video samples were inspected at full
   resolution; they support screen appearance and sequence, not input state,
   payment behavior, or exact frame cadence.

## Outcome

- **Code changed:** the startup coordinator carries completed New Game opening
  state into the field runtime; a Lua scene handler requests heading-zero
  movement through a typed native operation and suspends until the existing
  field controller consumes it after the first field-exit orchestrator pass.
  `michael_package.lua` resolves the first/repeat/ordinary quest branch;
  shared `motel.lua` supplies manager choice IDs. Typed C++ handlers still own
  movement execution, room/debt payment, and validated command execution.
  Loaded saves and ordinary field QA bootstraps do not arm the opening walk.
- **Behavior implemented:** New Game reaches the ROM C2 doorway with zero
  controller input, publishes event 2, and opens the first Stoker page. A
  enters the three-choice Michael-package offer; C opens the debt question; A
  pays through command `0x05` on the QA-seeded wallet, which applies the source
  `FFFC1E`, `FF018F`, runner-byte, and `FFFC30` mutations before showing the
  handoff page. A returns to the clean field. Typed story state selects the
  same package offer on an unpaid repeat and ordinary motel service after the
  debt and package flags are both complete. Physical repeat-entry movement
  and restored-save story/wallet state remain open.
- **Internally checked:** the Lua runtime, startup integration, Stoker route,
  field Hotel route, and dialogue runtime CTests pass in Debug and Release;
  both complete 92-test suites pass. The Lua runtime test checks the typed
  movement request, suspend/resume cycle, and two sequential native operations
  in one script; Stoker route QA asserts the handler completes only after the
  native auto-walk is consumed.
  They assert the completed-opening handoff,
  zero-input movement, collision bounds, raw and projected codes, mode/event
  state, indexed service selection, fade counters, the typed speaker, portrait
  alias and text, A/B/C choices, exact-250-nuyen payment, package handoff,
  post-payment story flags, transition suspension, audio-service transaction
  counts, and whole-screen return. The
   route writes 16 BMPs plus `route.tsv` and `source-boundaries.txt`. Every
  Debug BMPs were opened and reviewed at full size; the corresponding Release
   images are byte-identical. All 16 Debug and Release BMP SHA-256 hashes match.
- **Independently visually checked:** section 03 frames `000028..000055`,
  `000080..000082`, `000301`, `000601`, and `000690/000750/000780/000840`
  support the door/fade/panel appearance and the Stoker dialogue pages. The
  currently imported Stoker portrait crop matches the independent section 03
  portrait at `(16,32)`; the paid handoff has no independent video frame. These
  clips do not reveal controller input, collision data, or the source event.
  The sampled blank-shell timing remains open.
- **Reported defect resolved:** the physical scene-2 Stoker collision and
  automatic New Game arrival route are connected in the Qt conversion. General
  building access, same-archetype HUD palette/ammo, exact blank-shell timing,
  and actual audio playback are not closed by this hill.

## Lua manager-menu coroutine follow-up (2026-10-08)

This incremental scripting change leaves the manager page's pixels and
button-driven behavior unchanged. Shared `motel.lua` builds the ordinary motel
choices, adds the Michael quest's package choice when its location/state hook
allows it, and calls `game.ui.menu("manager", choice_ids)`. Native location
context supplies the text profile. The Lua task suspends while the existing
dialogue presenter displays the choices and receives A/B/C.
The selected stable ID resumes the task; the existing validated native
command handler still applies the choice's effect. The Stoker runtime supplies
the `stoker.motel` text profile. This is a connected generic menu operation,
not a general dialogue/window API, and item/audio/cutscene adapters remain
open.

1. **Does it look correct?** The manager page still shows the source A/B/C
   button rows and the three expected choices. A/B/C are direct action buttons;
   the ROM page does not use a movable row cursor, so no new highlight was
   added. The Debug capture matches Release by SHA-256, and independent section
   03 frame `000301` shows this manager screen family.
2. **Does the appearance reflect the behavior?** The unpaid Stoker location
   supplies `crash`, `leave`, and `package`; the page labels those choices, and
   A/B/C returns the corresponding stable ID to the suspended Lua function.
   The runtime test checks the menu request and returned `package` ID. The
   native command resolver still owns payment and other effects.
3. **Are the visuals correct?** The full manager screenshot retains the
   portrait, text frame, button glyphs, labels, margins, and black backing of
   the previous native page. The script path produces the same capture, so
   this change introduces no menu art or layout change.
4. **Is it animated?** The manager page is static while it waits for A/B/C.
   No timed row animation or cursor cadence was found in this menu path.
5. **Is there sound?** This coroutine adds no sound request. No distinct
   manager-choice sound call or playback duration is established by this
   route; mode-entry audio events remain a separate native service boundary.
6. **Are the other screen regions correct?** The full 320x224 manager capture
   contains no field map/HUD behind the panel and no second set of controls.
   The field return redraws the map and HUD cleanly.
7. **Do multiple frames support the interpretation?** The native route has
   sixteen whole-screen checkpoints from spawn through return. Independent
   section 03 captures include the manager page at `000301` plus the opening,
   debt, and referral pages. They corroborate screen family and content, while
   ROM/input-state evidence supports the action mapping.
8. **Was the previous interface cleared before this one appeared?** Yes. The
   transition goes through full black; the dialogue panel replaces the field
   scene, and the return redraws the field. The reviewed frames show no old
   buttons or graphics under the manager panel.
9. **Was the whole available sequence reviewed?** Yes. All sixteen Debug
   route images were reopened at their full 320x224 resolution. The same
   sixteen Release names and SHA-256 hashes match. Existing route QA covers
   New Game movement, collision, fade, dialogue choices, payment handoff, and
   field return; menu coroutine QA additionally checks suspension/resumption
   and selected-ID validation.

Debug and Release focused CTests pass for script runtime, dialogue runtime,
startup integration, and the Stoker collision-to-dialogue route. These are
conversion checks; the independent clip remains the visual reference, and no
claim of original-game sound timing is made here.

## Lua speaker/event dialogue follow-up (2026-10-08)

The first-visit interaction is now one readable Lua sequence:
`game.ui.say("arrival")`, then the shared
`motel.manager_interaction()` after the player presses A. The operation
resolves the native location text profile (`stoker.motel`) and speaker from
active location context, using only the event ID from Lua. `dialogue.lua`
supplies Stoker's name, matching text profile, and portrait alias; the runtime
validates that the selected speaker and location profiles agree. The text
resolver finds `stoker.motel.arrival.title` and
`stoker.motel.arrival.more`. Only this speaker/event is bound so far. The
dialogue presenter still draws the screen and owns input; the task resumes with
the manager menu after the source A continuation. Payment and inventory state
remain native.

1. **Does it look correct?** The arrival page shows Stoker, the extracted
   56x72 portrait, the sign text, and the centered A/`< MORE >` continuation.
   It matches independent section 03 frame `000082`. The same profile and
   event are carried by the typed script operation.
2. **Does the appearance reflect behavior?** The operation binds speaker
   `stoker` to its Lua speaker properties and joins the current
   `stoker.motel` text profile with event `arrival`. The page exposes one
   continuation option; B leaves it open, and A resumes the Lua task into the
   three-choice motel menu. The runtime tests check the request tuple and
   sequential say-to-menu continuation.
3. **Are the visuals correct?** The image remains the source-backed arrival
   panel and preserves its button glyph and label. Debug and Release route
   images are byte-identical after the presenter began resolving the page
   from the speaker/event request.
4. **Is it animated?** The speaker page is static while waiting for A. It adds
   no motion or timing loop; the preceding actor walk and fade remain the
   existing native services.
5. **Is there sound?** The dialogue request adds no audio operation. No
   discrete arrival-page or continuation sound and duration are established
   here; actual audio playback remains a separate open service.
6. **Are other screen regions correct?** The full-screen capture replaces
   the field map/HUD with the dialogue frame. After A, the manager page
   replaces the arrival text, and the eventual return redraws the field.
7. **Do multiple frames support the interpretation?** All sixteen native
   route checkpoints remain available; the independent video shows the
   arrival at `000082`, manager at `000301`, and other Stoker pages at
   `000601` and later frames. ROM and runtime input evidence establish the
   page transition; the video has no controller log.
8. **Was the old interface removed first?** Yes. The route fades to black
   before the dialogue frame replaces the field. The manager page is a full
   redraw, and the return reconstructs the map and HUD; no old buttons or
   graphics remain underneath the new panel.
9. **Was the whole sequence reviewed?** Yes. All sixteen Debug screenshots
   were reopened at 320x224; all sixteen Release names and SHA-256 hashes
   match. The arrival and manager handoff are checked by the live collision
   route and by the dialogue-runtime test, including the A-only wait.

The full Debug and Release CTest suites passed 92/92 in the prior route pass,
including the script, dialogue, startup, and Stoker route checks. This
connects the first Stoker dialogue event to Lua; additional speaker profiles,
dialogue events, item operations, audio, and cutscene placement/animation are
still unimplemented.

## Shared motel Leave dialogue through Lua (2026-10-08)

The generic `motel.lua` manager now sequences the ordinary Leave response:
after the player selects Leave, it asks the native dialogue presenter to show
the current location speaker's `leave` event, then returns the stable `leave`
choice ID. Stoker's source text is `stoker.motel.leave.title`; its Lua speaker
profile supplies the existing portrait. This reuses the shared motel
flow and location context rather than adding a Stoker-specific interaction
script. The ROM-derived prompt conversion already defines the matching
title-only `leave_notice` page and return to field.

1. **Does it look correct?** The full 320x224 farewell capture shows Stoker's
   portrait, the sourced sentence, the existing gray dialogue frame, and an
   empty lower choice area. It has no field map/HUD behind it.
2. **Does the appearance reflect behavior?** B selects the Leave row from the
   two-choice manager. Lua reads the speaker from native `game.location`,
   yields the `leave` text event, then returns the stable choice when the page
   is dismissed. The runtime and route checks verify the event/profile IDs,
   no-choice state, and field return.
3. **Are the visuals correct?** The farewell is title-only: there is no
   `< MORE >`, A/B choice row, selector, or leftover manager option. After
   dismissal, the map, HUD, and player position return.
4. **Is it animated?** No. This page adds no animation or timing loop.
5. **Is there sound?** The new Lua path adds no audio request. A source sound
   for this farewell and its duration are not established by this slice.
6. **Are other screen regions correct?** The ordinary two-choice menu, the
   farewell, and the field return were captured as full-screen images. The
   farewell page clears both previous menu rows; the field return redraws the
   HUD and playfield.
7. **Do multiple frames support the interpretation?** The complete Stoker
   route now has 16 Debug checkpoints from spawn through the scripted farewell
   and field return. Each Debug image was opened at original 320x224 size. All
   16 Release image hashes match. The independent video supports the broader
   Stoker screen family, but an exact farewell frame was not identified, so
   this is not claimed as independent pixel proof.
8. **Was the previous interface removed first?** Yes. The screenshot shows
   neither old manager text nor its option glyphs on the farewell page; the
   field image replaces the panel when the page closes.
9. **Was the whole available sequence reviewed?** Yes. All 16 current route
   images were reopened at full resolution, including both menus, the farewell,
   fade frames, and field return. Debug and Release hashes match. No new audio
   behavior or source sound duration is claimed.

Debug and Release pass the gameplay script, dialogue runtime, and full Stoker
route CTests. The route QA checks the farewell screen's full-screen redraw and
the clean return; the independent visual limit above remains open.

## Native story facts bound directly to Lua (2026-10-09)

The live field runtime now binds the allowlisted Stoker facts
`opening_dialogue_seen`, `michael_debt_paid`, and
`michael_package_collected` directly to its typed quest state. The script
runtime's `game.state` proxy reads and writes those properties through native
accessors. The field route no longer copies a `StoryFacts` snapshot into Lua
before opening the motel and then copies the result back afterward. The
dialogue adapter no longer sets the debt/package flags after payment either;
`michael_package.lua` updates them from the stable result returned by the
native payment service. Wallet, world-flag, and runner-record effects remain
inside the typed native service. A separate `open_stoker_interaction_for_dump`
entry keeps deterministic story fixtures confined to QA.

Debug and Release builds pass `shadowrun_gameplay_script_runtime_qa`,
`shadowrun_ghidra_field_dialogue_runtime`, and the physical
`shadowrun_ghidra_conversion_qt_field_stoker_collision_to_dialogue_qa` route.
The runtime QA verifies Lua observes a fact changed by native code and writes
the first-visit fact directly back. The physical route also verifies that the
Lua debt result updates the field's typed state before the handoff screen.
All 16 full-screen route BMPs match across Debug and Release by SHA-256. I
reviewed all 16 at 320x224; the unchanged field, dialogue, fade, handoff, and
return screens have no stale controls or overdrawn panels.

1. **Does it look correct?** The manager still shows the package choice while
   the debt/package facts are incomplete; the successful payment still shows
   the package handoff.
2. **Does the look match behavior?** The direct-state test and live route
   check native fact reads, the first-visit Lua write, and the payment result
   before the corresponding choice/page is presented.
3. **Are the visuals correct?** This change does not alter rendering, text,
   selection attributes, or panel geometry. The 16 full-resolution frames
   match between Debug and Release.
4. **Is it animated?** The refactor adds no animation. Field movement and
   fades retain their existing native update timing; dialogue pages remain
   static while awaiting input.
5. **Is there sound?** No audio request was added or removed by this change;
   it establishes no new sound cue or duration.
6. **Are other screen regions correct?** The reviewed captures include the
   full map and HUD, dialogue portrait and frame, menu rows, and return screen.
   The new path adds no graphics over the existing interface.
7. **Do multiple frames support this interpretation?** All 16 route
   checkpoints cover spawn, movement, arrival, both manager variants, debt,
   payment/handoff, leave, fades, and clean field return. They are native
   conversion captures, not independent Genesis behavior proof.
8. **Was the old interface cleared?** The same full-screen replacements and
   return redraw remain in use; no underlying menu or dialogue layer is left
   visible.
9. **Was the whole sequence reviewed?** Yes. All 16 BMPs were opened at
   320x224, and Debug/Release file hashes match. The independent longplay still
   supports the Stoker screen appearance, but does not expose controller or
   native story state.

Restored-save quest facts remain unresolved. The live field binding is an
explicit allowlist of typed properties; it does not expose arbitrary C++
objects or save memory to scripts.

## Speaker presentation profile moved to Lua (2026-10-09)

`gameplay/scripts/dialogue.lua` now owns the Stoker speaker's display name,
`stoker.motel` text profile, and `dialogue.portraits.stoker.motel` asset alias.
`game.location.speaker_id` selects the profile. Before opening a dialogue or
speaker-backed menu, the runtime checks that the speaker exists, that its text
profile matches location context, and that its portrait alias names an image
asset. The generic panel receives this resolved profile; its C++ code no
longer carries a parallel Stoker name/portrait table. The ROM-selected event
and source-derived page timing are unchanged.

1. **Does it look correct?** The Lua profile resolves to the same Stoker name
   and 56x72 portrait already shown by the arrival, manager, debt, and package
   pages. Current capture appearance matches independent section 03 frames
   `000082`, `000301`, and `000601`.
2. **Does appearance reflect behavior?** The physical scene-2 collision at
   `0x173138` selects the Stoker prompt through the source field-exit profile
   at `0x1F65A`; the active location supplies `speaker_id = stoker`. The new
   Lua profile must match its `stoker.motel` context and resolve a real portrait
   asset or the panel route stops at a source boundary. Debug and Release
   route tests exercise the collision-to-dialogue path and shared script menu.
3. **Are the visuals correct?** All 16 current full-screen checkpoints are
   320x224. Arrival, manager choices, debt, package handoff, referral, leave,
   fade, and field return were inspected after the profile move. The existing
   portrait, text positions, A/B/C markers, panel edges, map, and HUD remain
   intact.
4. **Is it animated?** The dialogue panel is static while waiting for input.
   This profile change adds no animation or timing; the automatic field walk
   and 17-update fade keep their existing native cadence.
5. **Is there sound?** The profile change adds no audio request and changes no
   playback duration. The route's separate source audio-service boundary is
   unchanged.
6. **Are neighboring screen elements also correct?** The whole-screen route
   includes the motel street, runner/HUD, dialogue frame and portrait, menu
   choices, and clean map return. The portrait and panel occupy the existing
   source-selected interface.
7. **Do multiple frames support the interpretation?** All 16 route BMPs were
   inspected; Debug/Release SHA-256 hashes match for every one. Independent
   longplay frames `000082`, `000301`, `000601`, and `000690` show arrival,
   manager, debt, and referral pages. That 30-fps clip supports visual order
   and appearance, not native input state or exact VBlank timing.
8. **Was old UI cleared before new controls were drawn?** The field fades to
   black before the dialogue panel replaces it. Manager choices replace the
   arrival page, and the return redraws the whole field/HUD; no previous
   buttons or graphics remain underneath.

The gameplay-script runtime, dialogue-panel test, field-dialogue runtime test,
and physical Stoker collision-to-dialogue QA all pass in Debug and Release
(4/4 each). This moves speaker presentation metadata for Stoker into Lua; the
other NPC profiles and their event dialogue remain unported.

## Explicit Stoker event and menu text bindings in Lua (2026-10-09)

The shared script UI now yields a stable event or menu ID without inventing
text keys from a naming convention. `dialogue.lua` explicitly maps the six
events used by the live Stoker scripts (`arrival`, `leave`, `room.paid`,
`room.insufficient`, `package-handoff`, and `jump-house`) to their title and
optional continuation keys. It separately maps the manager, room, and debt
menu IDs and each stable choice ID to a source text key under the
`stoker.motel` location-content profile. Menu copy is independent from the
optional speaker portrait, so the shared motel menu can be presented with or
without a named speaker. The text records remain the ROM-imported entries in
`assets/dialogue/script-text.lua`; this change moves their selection into the
gameplay script. Static Stoker prompts for the street ambush and package
question are still on the older native prompt path.

The runtime validates event keys, menu title keys, and one label key per
choice before the generic panel uses them. The script QA checks the explicit
arrival/farewell event keys, menu ordering and labels, and that the same motel
menu mapping works without a speaker ID. The physical route QA continues to
exercise the scripts through scene-2 collision, menu input, quest services,
and field return. Separate debt, referral, and low-funds routes verify the
native quest facts and corresponding full-screen debt/referral pages. Their
diagnostic helpers now seed the typed native story state that the live Lua
property binding reads, rather than writing the unused fallback map.

1. **Does it look correct?** The 16 full-screen 320x224 route captures still
   show the arrival, manager, debt, handoff, farewell, fade, and field screens
   with the same source text. Independent longplay frames `000082`, `000301`,
   `000601`, and `000690` show the arrival, manager, debt, and referral pages.
2. **Does the appearance reflect behavior?** The typed script operation
   carries the selected event/menu and stable choice IDs. Lua resolves those
   IDs to explicit content keys; tests cover choice order and content mapping.
   The manager offers package only on the unpaid quest branch.
3. **Are the visuals and selection attributes correct?** Existing panel art,
   portrait, text placement, A/B/C row markers, and source-backed page states
   are unchanged. This key-routing change adds no selector or decoration.
4. **Is anything animated?** Dialogue pages and choices remain static while
   awaiting input. The field approach and fade retain their existing cadence;
   this change adds no animation.
5. **Is there sound?** No sound operation or playback-duration behavior was
   added. Sound timing remains outside this slice.
6. **Are the other screen regions correct?** Every capture shows the whole
   game frame, including map/HUD, portrait, dialogue frame, and menu rows.
   The returned field view restores the runner and HUD.
7. **Do multiple frames support the interpretation?** All 16 route states
   cover spawn through clean return, and four independent longplay frames
   corroborate the key Stoker pages. The longplay does not expose native
   controller state or exact timing.
8. **Was previous UI cleared before new UI appeared?** The route fades fully
   to black before dialogue; each dialogue/menu state redraws its panel and
   choices. The farewell clears manager rows, and field return clears the
   dialogue panel. No old controls or graphics remain underneath.
9. **Was the full available sequence reviewed?** Yes. I reopened all 16 Debug
   BMPs at original 320x224 resolution. Each corresponding Release BMP has
   the same SHA-256 hash. The three debt/referral pages also match by
   Debug/Release SHA-256 and were opened at full resolution. The script
   runtime, generic panel, field-dialogue runtime, physical Stoker route, and
   all three follow-up routes pass in Debug and Release (7/7).

These checks validate the native conversion route and Lua content binding;
the independent capture remains the visual authority. The older static
street-ambush and package-question prompt paths still need to move into the
shared script flow.

## Full-screen review correction (2026-10-10)

Reviewing the complete arrival screenshot against section 03 frame `000090`
found a visual defect that the earlier pass missed: the source text was
`< MORE >`, but both glyphs pointed left in the Qt output. The field font
extractor had ignored the ROM HFlip bit on the `>` character-table entry. The
previous statements that the arrival glyphs visually matched were too broad;
the panel, portrait, and text placement matched, but the delimiter direction
did not. The importer now applies the flip bits for all field-font characters,
and the regenerated arrival screenshot has outward-facing `< MORE >` glyphs.
See the [font flip audit](dialogue-font-flip-orientation.md) for the ROM words,
pixel regression, and full-screen comparison.
