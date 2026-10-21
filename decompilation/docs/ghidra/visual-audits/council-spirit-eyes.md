# Council Island: Spirit Eyes and Licourtrix

Status: **partial**. Scene-4 event 27 now opens the Spirit Eyes Lua task from
ordinary movement, and the first-contact and horn-request frames are captured
as full 320x224 images. The directly traced ritual flags are represented in
Lua and the FFFC40 auxiliary marker is saved by the typed field state. The
Scene-47 conversation is wired through a separate event-25 entity link, but
its contextual physical route and full text graph are not defended. No
independent Genesis capture of either conversation has been identified.

## ROM trace

- Scene 4 record 96 (`0x15BDF6`) carries raw collision `0x00DB`, event 27,
  bounds x `0x06C0..0x06DF`, y `0x0080..0x009F`. It bypasses the ordinary event
  table and selects continuation `0x53D5A`; the event-table byte `0x80` is
  diagnostic data only. The figures at this location are composed map art,
  not default pedestrian records.
- `0x53D5A` opens text root `0xF4BD8`, sets story bit 5, and establishes the
  first FFFC30/FFFC40 ritual markers. The branch then requires Group Item bit
  4 (gargoyle horn) and bit 3 (hellhound pelt), consuming each and advancing
  story bits 6 and 7. Until story bit 8 is set, the final request is for a
  living feathered-serpent scale. Once bit 8 and Group Item bit 2 are both
  present, the handler consumes item bit 2, sets story bit 9, advances the
  ritual marker, and opens `0xF4C04`. The follow-up root `0xF4C18` supplies the
  post-ritual answer.
- Story indices 5 through 9 are fixed activity selections rather than rows in
  the live 23-entry `0x53A78` trigger table. Lua uses
  `StoryFlags.select_fixed_trigger` to preserve the `FFDD64` selection without
  falsely marking a live-table entry.
- The independent Scene-47 record 62 (`0x1D54B0`) is raw event 25. Its direct
  continuation `0x53EFA` is selected only for transition context `0x13`; the
  other route rewrites the event to 8. `0x53F00` selects story index 8 and
  opens Licourtrix's `0xF4C44` text profile. After the conversation,
  `0x53F50` calls `0x2196C` to set story bit 8 and clears Group Item bit 2.
  The conversation text says the serpent gives a scale, but no item-bit-2
  grant was found in this handler. Do not infer a grant from the text alone.

## Native and Lua implementation

- `gameplay/world/entities/council_spirit_eyes_site/links.tsv` owns Scene-4
  event 27 routing. `salish_spirit_eyes.lua` owns the ritual story gates,
  item consumption, FFFC30/FFFC40 changes, and prompts.
- The Scene-47 entity has a separate event-25 link. The host only reaches it
  when the ROM-derived context selects the direct continuation; ordinary
  event-25 rewrite behavior stays in the field-event policy.
- C++ exposes FFFC40 as saved `field_auxiliary_flags` facts. Older saves
  restore the new field as zero. Portraits are extracted from ROM profiles
  `0x121EF8` and `0x121EA4`, and loaded through the named Spirit Eyes and
  Licourtrix resource aliases.

## Verification

- Lua tests cover the site/event gates, the ordered horn and pelt handoffs,
  the final bit-2 gate, story-bit changes, and the Scene-47 post-dialogue
  clear without inventing an item grant.
- Debug CTest
  `shadowrun_ghidra_conversion_qt_field_spirit_eyes_physical_entry_qa` walks
  into record 96 with ordinary Up and checks the `0xFFDB`/event-27 dispatch,
  first-contact state, portrait alias, and horn request. It writes
  `qt_runtime/field-spirit-eyes-physical-entry-qa/actual.bmp` plus contact and
  follow-up frames. The full frames were inspected; their layout is an
  internal runtime capture, not independent proof of Genesis fidelity.

## Remaining evidence

- Trace every choice/response node in the `0xF4C44` graph against the ROM and
  establish the exact world-context condition for entering the direct
  continuation from Scene 47.
- Find the source path that grants Group Item bit 2, or confirm from the ROM
  that the dialogue path intentionally leaves the scale unavailable.
- Exercise the contextual Scene-47 route with physical input and compare both
  conversations against independent Genesis frames before raising the hill
  above partial.
