# Penumbra Fuchi scene-3/event-2 route

## ROM route and gate

Scene 3 collision record 97 at `0x18ABA0` has collision value `0x00C2`,
event 2, and bounds x `0x0080..0x009F`, y `0x0140..0x015F`. Event-table
row 2 selects resource activity scene 21 (`0x15`). Title service 14 at
`0x54152` displays the cyber-heart entry only when FFFC1E bit 25 is set, bit
14 is clear, and FFF0D5 is zero. It selects root `0x11B7F4`, page text
`0x11C213`, and portrait profile `0x122090`, then retains the event-table
destination.

The Lua link for `penumbra_site_02_fuchi` records the source conditions and
entry task in `gameplay/world/entities/penumbra_site_02_fuchi/links.tsv`.
`fuchi_offices.present_cyberheart_entry()` validates the building location
and story bits, marks trigger 14 active, says the extracted `entry` page, and
returns `enter_scene_21`. The active trigger index is separate from FFFC1E
bit 14, which remains clear as required by the route gate.

## Physical QA

The Debug and Release CTest `shadowrun_ghidra_conversion_qt_field_fuchi_offices_qa`
starts on scene 3 at `(0x0800,0x1700)` and moves Up through the field runtime.
It checks contact with record 97/event 2, the shared Lua entry-dialogue
boundary, extracted text, the Fuchi portrait profile, and the MORE prompt. A
controller A press closes the page and enters activity 21. The QA saves
full 320x224 frames at `spawn.bmp`, `doorway-contact.bmp`, `actual.bmp`, and
`activity-21.bmp`.

The full-screen Debug `actual.bmp` and `activity-21.bmp` were inspected. The
entry page shows the Fuchi logo portrait and its complete MORE prompt; the
activity-21 scene loads after A. This validates the host route and capture
dimensions, not visual equivalence to an independent Genesis capture. No
independent Genesis frame is available.

## Resource entry context

The ROM sets transient FFE187 on the gated title-service path at `0x54152`.
Resource setup at `0x167AE` consumes it by selecting resource index 2 and
marker ordinal 3, then clears the effect context. The conditional Lua link now
records `entry_resource_override=index_two_marker_three`; the host carries it
with the typed scene-21 route and applies it only after the `enter_scene_21`
dialogue result resumes the route. Debug and Release physical-entry QA now
asserts the resulting index and marker along with the activity capture. It
then invokes the typed resource-activity return adapter and checks the ROM
collision-probe lead point and away-facing heading in `source-field-return.bmp`.
That verifies the snapshot handoff; the in-activity exit trigger and
independent Genesis evidence remain open.
