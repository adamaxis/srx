# Renraku Offices scene-5/event-6 route

## ROM route and gate

Scene 5 collision record 46 at `0x067C36` has collision value `0x00C6`,
event 6, and bounds x `0x0260..0x027F`, y `0x00A0..0x00BF`. Its event-table
row 6 selects resource activity scene 35 (`0x23`). Title service 12 at
`0x54076` displays the entry passage only when FFFC1E bit 16 is set, bit 12
is clear, and FFF0D5 is zero. It selects dialogue root `0x110A2A`, entry
text `0x1195CF`, and portrait profile `0x121E2C`, then retains the event-table
destination.

The Lua link for `renraku_offices_frontage` records the route gate and entry
dialogue fields in `gameplay/world/entities/renraku_offices_frontage/links.tsv`.
`renraku_offices.present_map_overlay_entry()` validates the location and the
two story bits, marks trigger 12 as active, says the extracted `entry` page,
and returns `enter_scene_35`. Trigger 12 is the active dialogue trigger; it is
not a write to FFFC1E bit 12. The route keeps event-table scene 35 as its
typed destination.

## Physical QA

The Debug and Release CTest `shadowrun_ghidra_conversion_qt_field_renraku_offices_qa`
starts on scene 5 at `(0x2600,0x0D00)` and moves Up through the field runtime.
It checks contact with record 46/event 6, the shared Lua entry-dialogue
boundary, extracted text, the Renraku portrait, and the MORE prompt. A
controller A press closes the page and enters activity 35. The QA also checks
the scene and field-mode IDs and saves full 320x224 frames at `spawn.bmp`,
`doorway-contact.bmp`, `actual.bmp`, and `activity-35.bmp`.

The full-screen Debug `actual.bmp` and `activity-35.bmp` were inspected. The
dialogue panel shows the full lower text pane and the MORE prompt; activity 35
loads after the prompt. This validates the host route and capture dimensions,
not visual equivalence to an independent Genesis capture. No independent
Genesis frame is available. The FFE184 effect-context behavior and the
building's eventual return route remain separate open work.
