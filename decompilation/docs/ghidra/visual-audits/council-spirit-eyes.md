# Council Island: Spirit Eyes and Licourtrix

Status: **partial**. Scene-4 event 27 opens a JSON conversation graph from
ordinary movement. Debug/Release physical-input QA covers its complete known
first-contact prompt chain and saves full 320x224 images. The staged horn,
pelt, and scale gates are covered by Debug/Release script-runtime tests. The
completion path returns a typed cinematic request and `cinematics.json`
records its four ROM pages, but the field host does not yet present the
sequence. The Scene-47 Licourtrix text and branch graph now live in the JSON
conversation and condition catalogs, with Debug/Release runtime coverage for
the story and profile permutations. The active Qt coordinator now exercises
the seeded mode-table inputs through Scene-47 entry, including the ROM runner
spawn and five-actor Lua list. The upstream context producer and deferred
`0x53EE2` routine are still unresolved; record-62 physical input is not
exercised. The reviewed longplay sections contain no matching independent
Genesis dialogue frame.

## ROM trace

- The event-25 context is a result of the source mode table, not a fixed
  Scene-47 spawn value. At `0x2CE0`, FFFC10=`0x14` with FFFBF9=8 indexes
  `0x2E72` to select FFFC10=`0x13`; `0x2E42` maps that mode to Scene `0x2F`,
  and `0x2D92` supplies coordinates `0x5E80,0x1300`. If FFFC1E bit 8 is
  clear, this transition starts world activity with deferred routine `0x53EE2`
  and pending actor list `0x1E552`. Debug/Release `shadowrun_ghidra_field_mode`
  verifies both the unlocked activity handoff and the locked case. The seeded
  Qt mode-13 QA verifies coordinator selection, Scene-47 entry, spawn
  `0x5E80,0x1300`, and all five actors. It does not establish the upstream
  context producer or convert the deferred routine.
- Scene 4 record 96 (`0x15BDF6`) carries raw collision `0x00DB`, event 27,
  bounds x `0x06C0..0x06DF`, y `0x0080..0x009F`. It bypasses the ordinary event
  table and selects continuation `0x53D5A`; the event-table byte `0x80` is
  diagnostic data only. The figures at this location are composed map art,
  not default pedestrian records.
- First entry at `0x53D5A` initializes the FFFC30/FFFC40 ritual markers,
  writes `FFDD64=5`, and opens `0xF4BD8`. The first page is `0xF7204`; its
  `<MORE>` command `0x15` dispatches through `0x218B2`, calls `0x2196C` to set
  the active trigger bit, and selects prompt `0xF5150`. The linked prompts are
  the Spirit Eyes description `0xF72AB`, the one-choice welcome question
  `0xF735E` (label `0x103F4A`), response `0xF7401`, the one-choice ritual
  question `0xF74DD` (label `0x103F7B`), and the text-only first horn request
  `0xF7579`. Bit 5 is set only after the player advances past the opening
  page.
- On later visits, `0x53DA2` checks bit 6 and Group Item bit 4. Missing horn
  opens `0xF4BEC`; a present horn is consumed, sets bit 6, updates FFFC30 and
  FFFC40, then opens `0xF4C34` (`0xF7641`). The next stage checks bit 7 and
  Group Item bit 3: missing pelt opens `0xF4BF4`; a present pelt is consumed,
  sets bit 7, updates effects, and opens `0xF4C3C` (`0xF771D`). The scale
  stage shows `0xF7BC8` until story bit 8 and Group Item bit 2 are both set.
- With bit 8 and Group Item bit 2 present, `0x53E98` consumes the scale and
  `0x53EA8` sets bit 9 directly. The handler updates FFFC30/FFFC40 and opens
  `0xF4C04` (`0xF77FE`). Its command `0x1A` marks the already active trigger
  and selects prompt `0xF51A8` (`0xF78A3`). `<MORE>` dispatches command `0x4A`
  into `0x5D172`. It loads the resource descriptor at `0x1FE6C2` and passes
  the ordered pointers at `0xF4C20` to `0x5D1F6`. That blocking routine draws
  the four pages and waits for controller events before returning to the
  source field context. The graph reports
  `play_cinematic: salish.spirit_eyes.ritual_of_binding` with
  `source_context` return policy. The JSON catalog records page order and
  text keys; host resource extraction/presentation is still pending. The
  handler does not set `FFDD64=9` at this point.
- When bit 9 is already set, `0x53ED4` opens `0xF4C18`. The shared prompt
  runner sees its negative first-option sentinel, displays its first text-only
  entry `0xF7C79`, and returns. The adjacent `0xF7915`, `0xF798D`, `0xF79F0`,
  and `0xF7A8B` pointers are not consumed by this route in
  `0x1F246`/`0x1F3C4`; the separate `0x5D1F6` sequence consumes them in order.
- The independent Scene-47 record 62 (`0x1D54B0`) is raw event 25. Its direct
  continuation `0x53EFA` is selected only for transition context `0x13`; the
  other route rewrites the event to 8. `0x53F00` selects story index 8 and
  opens Licourtrix's profile before checking the story gates. If story bit 7
  is clear, it shows the flying-serpent text at `0xF7DBF` and returns. Only
  after bit 7 is set does it check bit 8; a set bit shows the empty-lair text
  at `0xF7E72`. With bit 7 set and bit 8 clear, the full conversation opens
  only when FFFC0E is nonzero, FFFC10 is `0x13`, and FFFBF9 is 8. The ordered
  prompt records begin at `0xF4C44` and link through `0xF51BC`, `0xF51D0`,
  and the A/B response loops at `0xF51E4`/`0xF51FC`. After the final prompt
  closes, `0x53F50` calls `0x2196C` to set story bit 8, then sets FFFC22 bit 2
  with `BSET #2,D0`. This is the Feathered Scale consumed by the Spirit Eyes
  event at `0x53E98` when Joshua returns.

## Native and Lua implementation

- `gameplay/world/entities/council_spirit_eyes_site/interactions.json` owns Scene-4
  event 27 routing. `dialogue/conversations/salish_spirit_eyes.json` owns the
  traced prompt chain, staged conditions, pages, and typed cinematic result.
  `dialogue/cinematics.json` owns the sequence's ordered page references and
  controller-event contract.
  `salish_spirit_eyes.lua` delegates both the ritual and Licourtrix encounter
  to their JSON graphs. `conditions.json` keeps the pelt-first/scale-second
  priority and the source profile/context gates as named, reusable facts.
- The Scene-47 entity has a separate event-25 link. The host only reaches it
  when the ROM-derived context selects the direct continuation; ordinary
  event-25 rewrite behavior stays in the field-event policy.
- C++ exposes FFFC40 as saved `field_auxiliary_flags` facts. Older saves
  restore the new field as zero. Portraits are extracted from ROM profiles
  `0x121EF8` and `0x121EA4`, and loaded through the named Spirit Eyes and
  Licourtrix resource aliases.

## Verification

- Debug/Release script-runtime tests cover the first-contact prompt states,
  command `0x15` timing, both one-choice inputs, missing and present
  horn/pelt/scale branches, item consumption, direct bit-9 completion,
  command `0x1A`, and the typed cinematic outcome. They also cover Licourtrix's
  ordered bit-7/bit-8 repeat pages, mode/profile gate, repeated lie loop,
  source choice labels, completion timing, and scale grant. Completed-task
  repeat shows the one known text-only page.
- Debug/Release CTest
  `shadowrun_ghidra_conversion_qt_field_spirit_eyes_physical_entry_qa` walks
  into record 96 with ordinary Up and checks `0xFFDB`/event-27 dispatch, the
  opening page before bit 5, command `0x15` and bit 5 after input, both
  one-choice questions, response pages, the first horn request, and clean
  Scene-4 return. It then requires the four cinematic text keys in ROM order,
  verifies the art rows against the named extraction, and advances with B,
  Left, Up, and Start. It writes a complete full-frame capture for every page
  and transition. The captures are internal runtime evidence, not independent
  Genesis fidelity proof.

## Remaining evidence

- Trace the upstream producer of context `0x14`, convert the deferred
  `0x53EE2` mode-exit routine into the dialogue/outcome model, and exercise
  Scene-47 record 62 with physical input. The active Qt route currently has a
  seeded QA entry; its JSON precondition exposes missing context as an
  unresolved boundary instead of treating it as a false story fact.
- The `0x1FE6C2` profile is extracted into a named host background and the
  four-page sequence uses the native full-screen presenter. Its route QA
  verifies source-context return. Independent Genesis dialogue captures
  remain unavailable, and the Licourtrix physical route is still open, so this
  hill stays partial.
