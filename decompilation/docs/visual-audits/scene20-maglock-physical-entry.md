# Scene-20 Maglock Physical Entry Audit

Date: 2026-10-07

**Status:** physical scene entry, maglock prompt, A continuation, and C/Walk
Away return are connected and pass Debug/Release QA. The exact scene-20
maglock screen has no matching independent capture in the reviewed longplay
corpus; do not claim independent visual fidelity. Passkey, Electronics,
grenade, Security Terminal, and other building outcomes remain separate.

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
choices; C (`0x20`) selects Walk Away and closes the panel. The route uses
ordinary input throughout and injects no collision, event, or menu state.

## Eight-question full-screen review

1. **Does it look correct?** I reviewed all 42 current 320x224 Release BMPs
   across the doorway approach, 17-frame fade, scene-20 entry and corridors,
   maglock contact, introduction, choices, and return. They show complete
   game screens rather than crops. The field HUD is present during play and
   absent under the full-screen dialog. Text and controls stay inside the
   shared panel; no prior button row remains underneath the choices.
2. **Does the appearance reflect the behavioral state?** The route trace
   records scene 2, the `0xC0` doorway contact and 17-frame fade, scene 20 at
   its descriptor position, record-117 contact with projected `0xFF82`, the
   open source-complete introduction, the three choice labels, and the closed
   panel after C. The active panel has no speaker actor, matching this
   property-triggered generic prompt.
3. **Are the visuals independently correct?** The ROM-backed assets and
   source records establish the screen content and placement inputs. The
   reviewed 320x224 longplay frames support the shared dialog shell and
   A/B/C-choice layout: section 03 frames `002100` through `002175` show a
   stable generic dialogue shell, and section 05 frame `000600` shows a
   three-choice Stoker panel. Neither is the scene-20 maglock event. No exact
   independent maglock introduction or choice frame was found, so its exact
   text/layout fidelity remains open.
4. **Is it animated, and when?** The field actor moves and changes walking
   pose while a direction is held. ROM-backed animation uses the four-pose
   walk list (`0xC062`) with the `0xBABC` increment `0x0280` against period
   `0x1000`; the one-pose idle list (`0xC052`) returns to frame 0. The scene-20
   idle images at entry, frame 8, and frame 30 are byte-identical, so no idle
   animation plays in this sample. Exact Genesis input/frame phase is not
   available from the longplay.
5. **Does sound accompany it?** `audio-events.txt` is empty for this route;
   the Qt path makes no audio request while opening the prompt or choosing
   Walk Away, so there is no native clip duration to report for those steps.
   This does not prove the ROM is silent during movement. Passkey/Electronics
   success and grenade/security effects are not exercised here; their mapped
   resources and unresolved stop behavior remain tracked in the separate
   interaction/audio audits.
6. **Are other screen areas correct too?** The review includes the full
   field, right HUD, fade, corridor, dialog border, portrait well, text well,
   and every option row. Debug and Release generated the same pixel hashes
   for every current full-screen BMP.
7. **Do multiple frames support the interpretation?** The QA writes 42 full
   frames, 34 of them pixel-distinct. Scene-20 entry, frame 8, and frame 30
   confirm the static idle state; corridor and approach checkpoints show
   movement; intro and choice images show the page replacement. Longplay
   frames 03/002100..002175 and 05/000600 support the shared dialog family,
   not the exact maglock event.
8. **Were old controls or graphics cleared?** The introduction replaces the
   field composition; A replaces the introduction with the three-choice
   page; C closes it. After C, the rendered 320x224 image is pixel-identical
   to `paint_field_scene_for_dump()` at the same actor and camera state. This
   confirms that the dialog, old buttons, and fade are absent. It is an
   internal leftover-clear check, not independent Genesis visual evidence.

## Reproduction and artifacts

Run `shadowrun_ghidra_conversion_qt --assets decompilation/assets
--field-scene20-physical-entry-qa <output-dir>`. CTest name:
`shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`. The QA owns
and refreshes the named output directory before writing. It emits `actual.bmp`,
`scene20-entry.bmp`, movement/transition checkpoints, `maglock-contact.bmp`,
`maglock-introduction.bmp`, `maglock-choices.bmp`,
`scene20-return-after-walk-away.bmp`, `route.tsv`, `source-boundaries.txt`,
and `audio-events.txt`.

Debug and Release outputs are under
`build/conversion/qt/{debug,release}/qt_runtime/field-scene20-physical-entry-qa/`.
All 42 BMP files match by SHA-256 across those configurations. The native
captures verify the implementation route and redraw behavior; they are not a
visual golden for the Genesis maglock scene.
