# Gates Undersound: scene 0, event 3

Status: **partial**. The ROM-gated Ogami departure service is now reached by
ordinary field movement and returns to the same scene after its one-page
narrative. Debug and Release captures are whole 320x224 frames. No independent
Genesis capture of this branch has been identified, and the other Gates
Undersound service paths remain open.

## ROM trace

- Scene 0 collision record 45 is at `0x0D57A0`, covers map cells `(29,3)`, and
  publishes raw event `0x00C3` / event 3. Event-table row 3 at `0x0D54B5`
  selects resource activity 21 on the generic route.
- Indexed title service 15 at `0x54222` requires scene/event 0/3, FFFC1E bit
  11 set, bit 15 clear, alternate result zero, and FFF0D5 zero. It selects
  root `0x1111DE` with portrait profile `0x121FC4`.
- The service has no selectable option. Its single page at `0x113D46` says
  that Ogami has left for Matchsticks. The result sets FFFC30 bit 27, clears
  bit 26, marks trigger 15, and returns D6=-1. It stays in scene 0 and
  suppresses the generic activity-21 destination.

## Lua route and host behavior

`gameplay/world/entities/downtown_site_04_gates_undersound/interactions.json` carries
the default event-table row and a higher-priority, story-gated service row.
`building.lua` resolves the service task from that entity data.
`gates_undersound.lua` is a small route adapter into the JSON graph. The graph
rechecks its named location/event/story precondition, begins trigger 15, shows
the ROM-backed page, applies the two ordered FFFC30 writes, finishes the
trigger, and returns typed `continue_in_place`. The generic field presenter
owns page input; the outcome dispatcher suppresses the event-table activity
and resumes the current field scene.

## Validation

- `gates_undersound_tests.lua` checks both story-bit gates, alternate result,
  one-shot flag changes, and the typed same-map outcome.
- `dialogue_graph_tests.lua` covers reusable pages, choices, branches, actions,
  typed outcomes, and unknown-condition boundaries. The C++ runtime and field
  dialogue tests advance this graph through its page and verify the typed
  presenter outcome.
- `shadowrun_ghidra_conversion_qt_field_gates_undersound_qa` starts at
  `(0x3A00,0x0A00)`, enters record 45 with ordinary Up input, and verifies the
  event, title-service page, story bit 15, FFFC30 bits 26/27, and same-scene
  return. Debug and Release write `actual.bmp`, `field-return.bmp`, collision
  evidence, and a route trace under
  `ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-gates-undersound-qa/`.
- The Debug `actual.bmp` and `field-return.bmp` were inspected at native
  resolution. This checks the complete frame, not independent Genesis pixel
  fidelity.

The Renraku and Fuchi entry pages still need ordinary field-input QA. Their
dialogue continuations and source effect bytes remain partial.
