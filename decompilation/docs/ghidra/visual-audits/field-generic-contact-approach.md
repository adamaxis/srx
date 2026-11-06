# Generic field NPC contact: first-overlap dispatch

Status: **partial**. The source trigger and a seeded, controller-driven host
approach are covered. A natural Genesis approach capture is still missing.

## ROM contact trigger [C]

The US ROM SHA-256 is
`92a614491fc3ea12a6df32459cce08b48574b7b814e244c07eb860dc5c6edb7e`.
Actor-pair service at `0xEB92` scans later active records. Unlinked pairs use
the half-open `[-0x40,+0x40)` window on both axes (`0xEC1C..0xEC46`); linked
pairs use `[-0x100,+0x100)` (`0xEBEA..0xEC18`). On a resolved overlap,
`0xEC8C` calls `0xECCC` in the same service pass.

At `0xECCC`, the candidate actor's `+0x9E` bit 4 is checked, then the other
record's `+0x10` bit 1; the symmetric bit-4 check is at `0xEDDC..0xEDF0`.
When the global eligibility byte `FFE176` is clear, the ROM selects callback
`0x21174` at `FFDD2C` and calls `0x1E2E0`. If the shared `FFE11C` gate is zero,
`0x1E2EC` loads `0x3C` (60) and copies the selected callback to `FFE104`.
`0x1DD94` decrements `FFE11C` during the field service.

There is no actor-local dwell counter or contact-break reset before callback
selection in this path. The 60-frame `FFE11C` value is a shared activity gate
armed after an eligible overlap; it is not a 1–2 second pre-contact timer.
Special bit-3 contacts take a separate branch at `0xED3C..0xEDAC`.

## Scene-10 source data and host path

The Scene-10 default list at `0x17034` points to position row `0x1704A`, whose
coordinates are `(0x0700,0x0500)`. The imported live actor has `+0x9E` bit 4
set and selects the Lua `field_generic_contacts.present` route through its
stable activity ID. Ricky and hireling Marrs use that data-driven generic
route in the bulk QA. Ilene's extraction contact is different: its bit-3
branch selects `0x54402` and has its own story activity.

`shadowrun_ghidra_conversion_qt_field_scene10_generic_contact_input_qa` now
starts the runner at a documented seeded host point `(0x0500,0x05AA)`, faces
right, and drives ordinary Right input. It does not write the NPC overlap or
callback state. The trace enters the source window at update 18 at
`(0x06C2,0x04D8)`; the ROM actor is at `(0x0700,0x0500)`. After stopping input,
the generic response appears 22 updates later. The QA checks the source text,
single dialogue-open event, 59-frame host activity count, and writes
`actual.bmp` plus `input-route.tsv` under the Debug/Release build directories.

The seeded player position and facing isolate the approach path; they are not
claimed as a natural game-entry state. `actual.bmp` is an internal host dump,
not an independent Genesis visual comparison. No gameplay runtime code
changed in this slice; it adds input-to-contact regression coverage around
the existing Lua route.

## Video and remaining evidence [H]

The reviewed 30 fps section-04 frames show the receptionist's package page at
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/section_04_field_corridor_npc_dialogue_0238-0390/frame_000600.png`
(source time 00:04:18), followed by field movement. That sequence does not
identify the Scene-10 bit-4 actor or establish a contact dwell interval. No
natural Ricky, Marrs, or extraction-target contact was matched to the ROM path
in the reviewed frames.

The saved-state probe `gameplay/tests_lua/tools/measure_bump_contact_rom.py`
could not be rerun in this checkout: `emu/doors/jh3.state` is absent and
`GPGX_CORE` is unset. Prior progress notes record a forced-overlap GPGX
first-text measurement; its raw measurement output is not present here, so
this turn does not claim a fresh emulator timing comparison. Keep playtest
#28b open until a natural actor approach is captured and matched in the
Genesis footage or emulator.

## Validation

Headless Debug and Release CTest each passed these three cases:

- `shadowrun_ghidra_conversion_qt_field_scene10_generic_contact_qa`
- `shadowrun_ghidra_conversion_qt_field_scene10_generic_contact_input_qa`
- `shadowrun_ghidra_conversion_qt_field_bulk_generic_contacts_qa`

These are focused host checks; they do not constitute project-wide QA.
