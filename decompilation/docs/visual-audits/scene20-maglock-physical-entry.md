# Scene-20 Maglock Physical Entry Audit

Date: 2026-10-07

**Status:** physical scene entry, maglock prompt, A continuation, C/Walk Away,
and B/no-grenade result with A dismissal are connected and pass Debug/Release
QA. The exact scene-20 maglock screen has no matching independent capture in
the reviewed longplay corpus; do not claim independent visual fidelity.
Passkey, Electronics, grenade consumption/security effects, Security Terminal,
and other building outcomes remain separate.

## ROM route and native state

The US ROM scene-2 collision stream at `0x173124` contains records 89 and 90:
two adjacent `0xC0` rectangles covering the same doorway lane at
`x=0x0E0..0x11F`, `y=0x260..0x27F`. Event-table row 0 at `0x172CDC` is
`0x14`, so ordinary D-pad contact takes the source fade and enters scene 20.
The New Game actor begins at fixed position `(0x3CE0, 0x2D00)`; scene 20's
source descriptor starts it at `(0x5A00, 0x0800)`.

The scene-20 profile row at `0x3282` points to the collision stream at
`0x1D9292`. Record 117 is at `0x1D9724`; its bytes are
`04 60 04 7F 00 E0 00 FF 00 82`, meaning inclusive bounds
`x=0x460..0x47F`, `y=0xE0..0xFF`, value `0x82`. Normal Up reaches contact at
fixed position `(0x4718, 0x1048)`. The typed interaction publishes projected
code `0xFF82` and activity selection 2; the handler opens the source-backed
maglock introduction. At this point the raw terrain sample is already clear
(`0`), so the QA checks the retained projected object code rather than
mislabeling it as the raw terrain sample.

The route exposed three connected-state issues. The native collision path did
not publish every in-bounds raw terrain sample or release B7B8's one-shot
route bit after returning to clear terrain. The world-activity timeout was
not tied to the `0x1DDF0` field-frame countdown. Scene transition re-entry also
left the prior display-service gate set, despite ROM `0x08C4` clearing it after
the new scene buffers are prepared. The typed field runtime now follows those
three source transitions, allowing the next scene event to dispatch.

The dialog remains the shared generic field panel. The no-passkey property
selects its introduction text; A (`0x40`) advances to the three source-backed
choices; C (`0x20`) selects Walk Away and closes the panel. A second ordinary
route waits the source 10 frames, then B (`0x10`) selects Frag Grenade while
the active New Game party has no item code `0x0D`. The generic no-item result
uses the source text, has no choices, waits 30 frames, and closes on A. Both
routes use ordinary input throughout and inject no collision, event, or menu
state.

## Eight-question full-screen review

1. **Does it look correct?** I reviewed all 47 current 320x224 BMPs across
   the doorway approach, 17-frame fade, scene-20 entry and corridors, maglock
   contact, introduction, choices, no-item result, wait, and return. They show
   complete game screens rather than crops. The field HUD is present during
   play and absent under the full-screen dialog. Text stays inside the shared
   panel; the result replaces all three old choice rows.
2. **Does the appearance reflect the behavioral state?** The route trace
   records scene 2, the `0xC0` doorway contact and 17-frame fade, scene 20 at
   its descriptor position, record-117 contact with projected `0xFF82`, the
   open source-complete introduction, the three choice labels, a newly pressed
   B edge (`0x1000` high byte), no active-party Frag Grenade, the generic
   information result, and the closed panel after A. Both pages have no
   speaker actor, matching this property-triggered generic prompt.
3. **Are the visuals independently correct?** The ROM-backed assets and
   source records establish the screen content and placement inputs. The
   reviewed 320x224 longplay frames support the shared dialog shell and
   A/B/C-choice layout: section 03 frames `002100` through `002175` show a
   stable generic dialogue shell, and section 05 frame `000600` shows a
   three-choice Stoker panel. Neither is the scene-20 maglock event. No exact
   independent maglock introduction or choice frame was found, so its exact
   text/layout fidelity remains open.
4. **Is it animated, and when?** The no-item result is static during its
   source 30-frame wait: frames 1, 15, and 30 and the held frame have identical
   pixels. The field actor moves and changes walking pose while a direction is
   held. ROM-backed animation uses the four-pose
   walk list (`0xC062`) with the `0xBABC` increment `0x0280` against period
   `0x1000`; the one-pose idle list (`0xC052`) returns to frame 0. The scene-20
   idle images at entry, frame 8, and frame 30 are byte-identical, so no idle
   animation plays in this sample. Exact Genesis input/frame phase is not
   available from the longplay.
5. **Does sound accompany it?** Both route `audio-events.txt` files are empty.
   The no-item branch issues no audio request in the Qt path; the available-
   grenade consumption/security path is separate. There is no clip duration
   to report for the no-item response. This does not prove the ROM is silent
   during field movement. Passkey/Electronics success and grenade
   consumption/security effects remain tracked in the interaction/audio
   audits.
6. **Are other screen areas correct too?** The review includes the full
   field, right HUD, fade, corridor, dialog border, portrait well, text well,
   and every option row. Debug and Release generated the same pixel hashes
   for all 47 BMPs in the B/no-item route and all 42 in the C/Walk Away route.
   The 38 same-named pre-result screens are also pixel-identical between
   routes.
7. **Do multiple frames support the interpretation?** The C route writes 42
   full frames, 34 pixel-distinct. The B route writes 47, 35 pixel-distinct.
   Its three timed result captures and held image confirm a stable wait;
   corridor and approach checkpoints show movement; intro and choice images
   show page replacement. Longplay frames 03/002100..002175 and 05/000600
   support the shared dialog family, not the exact maglock event.
8. **Were old controls or graphics cleared?** The introduction replaces the
   field composition; A replaces it with the three-choice page; B replaces
   all choice rows with the no-item message; A closes that message. After A,
   the rendered 320x224 image is pixel-identical to
   `paint_field_scene_for_dump()` at the same actor and camera state. This
   confirms that the dialog, old buttons, and fade are absent. It is an
   internal leftover-clear check, not independent Genesis visual evidence.

## Reproduction and artifacts

Run `shadowrun_ghidra_conversion_qt --assets decompilation/assets
--field-scene20-physical-entry-qa <output-dir>` for the C/Walk Away route, or
`--field-scene20-grenade-no-item-qa <output-dir>` for the B/no-item route.
CTest names are `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`
and `shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa`. Each QA
owns and refreshes its named output directory before writing. The new route
emits `actual.bmp`, `maglock-grenade-unavailable.bmp`, three timed wait frames,
`maglock-grenade-result-held.bmp`, both full-screen return comparisons,
`route.tsv`, `source-boundaries.txt`, and `audio-events.txt`.

Debug and Release outputs are under
`build/conversion/qt/{debug,release}/qt_runtime/field-scene20-physical-entry-qa/`
and `field-scene20-grenade-no-item-qa/`. The 42 C-route BMPs and 47 B-route
BMPs match by SHA-256 across configurations. The native captures verify the
implementation route and redraw behavior; they are not a visual golden for
the Genesis maglock scene.
