# Scene-10 northwest restaurant threshold event 30

Scene-10 collision record 11 at ROM `0x1D618C` has raw value `0x019E` and
covers map pixels `(0,32)..(31,63)`. `0xB7B8` selects the direct event-30
continuation at `0x20990`; Scene 10 reaches default branch `0x20C78`, which
loads portrait profile `0x121E5C` and prompt `0xF3FAC`. Its first text pointer
is `0xF3FBC` and its first choice pointer is `0xFFFFFFFF`. No scene-specific
gate or map destination is present in this branch. The entity link is
`restaurant_northwest_threshold`, with the shared unnamed old-man profile.

Debug/Release CTest
`shadowrun_ghidra_conversion_qt_field_scene10_northwest_contact_qa` checks the
descriptor spawn `(240,204)`, then starts from a seeded position `(16,80)`
below the collision rectangle and uses ordinary Up. Record 11 dispatches event
30 on update 5; Lua opens the five-segment extracted passage with zero options.
Ordinary A dismisses it, and the test checks the 60-frame same-scene return
and visible field frame. Both builds write eight full-frame captures and
`route.tsv` under
`build/conversion/qt/{debug,release}/qt_runtime/field-scene10-northwest-contact-qa/`.
The Debug dialogue and returned-field images were inspected; the eight images
match byte-for-byte across configurations. This is host regression evidence,
not an independent Genesis comparison. Keep the contact partial until a
matching real-game frame is identified.
