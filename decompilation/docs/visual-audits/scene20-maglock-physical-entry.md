# Scene-20 Maglock Physical Entry Audit

Date: 2026-10-08

**Status:** The ordinary scene-2 doorway and scene-20 lock trigger reach the
shared Qt interaction. Physical QA covers Walk Away, unavailable grenade,
passkey success and insufficiency, no Electronics skill, a positive-skill
partial result, kit-assisted success, and the Electronics zero-roll alert.
The alert result and its `0x78`-frame scene timeout are connected through the
live field world-frame state. The source entity-target selector remains an
explicit boundary, so downstream alert actors/effects are still open. The exact
maglock screen has no matching independent Genesis capture in the reviewed
longplay; do not claim independent visual fidelity or story-driven item
acquisition. Grenade consumption/security effects, other passkey qualities,
and the Security Terminal outcome remain open.

## ROM route and native state

Scene-2 records 89 and 90 at `0x173124` are paired `0xC0` doorway rectangles.
Event-table row 0 at `0x172CDC` selects scene `0x14`; ordinary input takes the
source fade from New Game position `(0x3CE0, 0x2D00)` to scene 20's descriptor
position `(0x5A00, 0x0800)`.

Scene-20 record 117 at `0x1D9724` is value `0x82`, with bounds
`x=0x460..0x47F`, `y=0xE0..0xFF`. Normal Up reaches it at fixed position
`(0x4718, 0x1048)`, publishes projected code `0xFF82` and event selection 2,
and opens the source-backed generic maglock prompt. The raw terrain sample is
clear at contact; the QA checks the projected object code, not the raw sample.

The `shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa` route
uses the ordinary New Game Decker (template 1, class 3, Electronics 1, no
Electronics Kit or passkey). ROM `0x21692` sets the no-kit target to 5 and runs
one trial. Test-only seed `0x0006` produces a single face 5 through `0x0D44`,
`0x0D3A`, and `0x0DBA`. That success reduces `FFE16E` from 2 to 1 and selects
the source partial-result text at `0x11CD42`; it adds no action sound and does
not change the collision map. The result page is held for 30 frames and
ordinary A returns to the field.

The physical retry is a fresh collision. ROM `0xB994` writes the collision
event value back to `FFE16E`, so ordinary Up opens the same lock with selection
2 again. The QA asserts this reset; it does not carry partial progress across
field re-entry. The route then adds item `0x15` to the live Decker inventory
as an explicit test fixture. With Electronics 1 plus the kit, ROM uses three
trials against target 3. Test-only seed `0x0004` yields faces 4, 2, 4, giving
two successes and selecting the success result at `0x11CD3A`. The handler sets
`FFE16E=0xFF`, calls `0x21616`, and requests sound `0x4B`. The result screen
itself leaves map index 483 at `0x82`; after its 30-frame wait and ordinary A,
`0x2BD2` commits `0x82 -> 0x81`. The full return frame equals the field-only
render. The kit grant is QA-only; normal kit acquisition remains open.

The other physical routes cover the shared prompt with New Game state or
explicit test fixtures. Passkey quality 2 succeeds and preserves item `0x14`;
quality 1 against selection 2 keeps the generic three-choice page without
sound or map change. The New Game Samurai's skill byte `+0x8A=0` selects the
no-skill string before any roll. With no active Frag Grenade (`0x0D`), B shows
the generic unavailable-item result. All use ordinary field input and check
the complete 320x224 scene/HUD or modal redraw. The existing route also fixed
the raw terrain publication/one-shot B7B8 retry bit, connected the
world-activity timeout, and releases the prior display gate after scene
buffers are prepared.

The `shadowrun_ghidra_conversion_qt_field_scene20_electronics_alert_qa` route
uses the ordinary New Game Decker with no kit or passkey. ROM `0x21692` runs
one no-kit trial against target 5; source PRNG seed `0x0000` yields face 1 and
branches through `0x21740` to `0x1E04A` and alert text `0x11CD4A`. Because the
live scene is `0x14`, `0x1E04A` calls `0x1DBAA`, which arms `FFDF84=-1` and
`FFDF86=0x78`, then returns without requesting sound `0x30`. The native
interaction writes those typed timeout words into the FieldSceneRuntime's
world-activity state. A full field frame after ordinary A acknowledgement
passes `world_active=0xFFFF` to ROM-derived `0x1DE00` logic; scene `0x14` takes
the entity-target selection branch and leaves the cleanup countdown at 120.
That selector callback is still unresolved, so this verifies the state and
handoff but not an actual spawned security actor or alarm effect.

The alert page preserves selection 2, inventory, and the `0x82` collision map;
it requests no sound during the action and holds unchanged across the source
30-frame result wait. Ordinary A clears the modal page and restores a complete
field/HUD frame equal to the field-only render.

## Eight-question full-screen review

1. **Does it look correct?** The six earlier routes contain 286 BMPs; the new
   alert route contains 48 BMPs, with 37 distinct full-screen frames. I reviewed
   all 37 alert-route frames in contact sheets and inspected the alert result,
   its held frame, and clean field return at native 320x224 resolution. The
   result uses the shared full-screen panel, fits both text segments inside the
   text well, and has no option rows or field HUD underneath.
2. **Does the appearance reflect the behavioral state?** Traces and assertions
   check scene 2 C0 entry, scene-20 record 117, projected `0xFF82`, selection
   2, live runner skill/inventory, the correct prompt/action, result text,
   inventory, collision map, sound request, and clean field return. The
   zero-roll trace checks Decker skill 1, no kit/passkey, seed 0, unchanged
   selection/inventory/map, timeout marker -1, and 120 frames. For
   Electronics, the partial result holds selection 1 only until field return;
   the next physical contact resets it to 2 per `0xB994`. Kit success sets
   selection `0xFF`; map index 483 changes only after the result is dismissed.
3. **Are the visuals independently correct?** The supplied longplay has no
   exact scene-20 maglock introduction, choice, or result frame. Sections 03
   and 05 support the shared dialogue shell and A/B/C choice family, but cannot
   prove this event's exact text/layout. The ROM-backed assets establish inputs
   to the conversion; internal captures are not a Genesis golden.
4. **Is it animated, and when?** The introduction holds through its 10-frame
   delay. Choices remain pixel-identical across an 8-frame hold. No-skill,
   zero-roll alert, partial, kit-success, passkey, and unavailable-item result pages are static
   at frames 1, 15, 30, and held; ordinary A then dismisses each result. The
   field runner changes walk poses while moving; sampled scene-20 idle frames
   are identical. Exact Genesis input/VBlank phase is unavailable.
5. **Does sound accompany it?** The no-skill, alert, partial, insufficient-passkey,
   unavailable-grenade, and Walk Away branches add no action sound. Passkey and
   kit-assisted Electronics success request `0x4B`, resolved to resource 075
   (97,770 samples at 44,100 Hz, 2.217 seconds). QA verifies the request, not
   route-specific speaker loopback or playback tail. On the scene-20 zero-roll
   branch ROM calls `0x1E04A`, whose scene-`0x14` path only schedules the
   timeout; the tested action adds no sound request, including `0x30`.
6. **Are other screen areas correct too?** The review covers the whole map,
   right HUD, transition fade, player, panel frame, portrait well, text well,
   option rows, and result/return composition. All seven Debug and Release sets
   match by relative-path SHA-256: 334 BMPs per configuration, zero
   mismatches. The routes contain 43 physical-entry, 48 no-grenade, 46
   passkey-success, 45 insufficient-passkey, 48 no-skill, 48 alert, and 56
   Electronics-trial captures.
7. **Do multiple frames support the interpretation?** The earlier 77 distinct
   images include route approaches, fade phases, field movement, the introduction,
   choices, result entry/holds, and clean returns. The alert route's 37 distinct
   images include its approach, transition, choices, alert result, held result,
   and clean return. Electronics result checks
   compare first, 15th, 30th, and held frames. Debug/Release hashes match for
   all 334 captures. Independent longplay frames support only the shared dialog
   family, not the maglock event itself.
8. **Were old controls or graphics cleared?** Result pages have no prior
   choice rows. After dismissing a result, the full scene render equals the
   field-only render at the same state; HUD and player are visible, with no
   dialog or stale buttons. This is an internal leftover-clear check, not
   independent Genesis visual evidence.

## Reproduction and artifacts

CTest names are `shadowrun_ghidra_conversion_qt_field_scene20_physical_entry_qa`,
`shadowrun_ghidra_conversion_qt_field_scene20_grenade_no_item_qa`,
`shadowrun_ghidra_conversion_qt_field_scene20_passkey_qa`,
`shadowrun_ghidra_conversion_qt_field_scene20_insufficient_passkey_qa`,
   `shadowrun_ghidra_conversion_qt_field_scene20_electronics_no_skill_qa`,
   `shadowrun_ghidra_conversion_qt_field_scene20_electronics_alert_qa`, and
   `shadowrun_ghidra_conversion_qt_field_scene20_electronics_trials_qa`. The
   positive-skill and alert routes are also available as
   `--field-scene20-electronics-trials-qa` and
   `--field-scene20-electronics-alert-qa <output-dir>`.

Debug and Release artifacts are under
`build/conversion/qt/{debug,release}/qt_runtime/` in the corresponding
`field-scene20-*-qa/` folders. The seven route outputs contain 334 BMPs; the
new alert route has 48 BMPs and 37 distinct frames. Debug/Release relative-path
hashes match for the new route. Trial QA pins the PRNG only at action
boundaries and seeds item `0x15` only in the QA runner. These captures verify
the connected native route and clean redraw, not independent Genesis visual
fidelity.
