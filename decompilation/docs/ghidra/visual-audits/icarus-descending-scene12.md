# Icarus Descending (field scene 12)

Status: partial. The map viewer has source-linked room fixtures and Caleb's
text/portrait profile. Both the bodyguard rejection and the accessible Caleb
branch now use the scene entity's Lua link. The first-meeting, repeat, and
post-run conversation roots are connected through the shared Johnson system;
the information/contact branch, full run outcome, and an independent Genesis
capture of the scene remain open.

## ROM and route trace

- Scene 0 event record 53 at `0x0D57F0` is event 1. Its route-table byte is 1
  and the traced handler path selects field scene 12. Mike's Downtown marker 8
  supplies the `Icarus Descending` map-name crosswalk. The guide's reputation-6
  condition is not yet tied to a specific ROM flag writer.
- Scene 12 event records 9 and 10 at `0x1D604A` and `0x1D6054` are event 8/9
  cells at `(96,224)` and `(64,224)`, with raw value `0x0080` and no event-table
  byte. Their player-facing behavior and destination stay unresolved.
- Scene 12 record 11 at `0x1D605E` is event 30 at map pixel `(0,32)`. ROM
  `0xB7B8` sends it to `0x20990`. The scene branch checks `FFFC1E` bit 10. With
  the bit clear, it displays descriptor `0x1056A4` / text `0x108796`, where
  two bodyguards block the booth. With the bit set, the branch uses dialogue
  slot 4 and profile sprite `0x121E98` for Caleb Brightmore.
- The bit-clear branch now resolves through the reusable scene-entity link on
  `icarus_descending_booth` to the Lua task
  `icarus_descending.present_access_blocked`. The task checks event 30 and bit
  10 again, then presents the bodyguard-blocked page without changing story
  state. The embedded script-runtime test covers route selection and text. The
  physical-input QA reaches this branch from south of the event rectangle with
  ordinary Up movement and checks that the access bit, referral flags, and
  runner counters stay unchanged.
- With bit 10 set, the same entity link selects `johnson.interaction` and
  Caleb's location profile. ROM `0x20A78..0x20AA4` sets FFFC27 bits 4 and 7
  and applies `+2` to each of the three runner source-byte records on first
  contact. Lua records those bits as the access/contact and first-visit facts,
  presents the two first-meeting pages, then opens the three-choice root.
  Repeat visits use the regular opening; the source post-run gate selects the
  post-run root. The work choice reaches the shared offer system with context
  4 and Caleb's source work-opening page.
- `shadowrun_gameplay_script_runtime_qa` now verifies the accessible route,
  first-visit service request, both pages, root choices, leave page, persisted
  facts, and context 4 in Debug and Release. This is script-runtime coverage;
  the dedicated physical-entry CTest below covers the accessible event path.
- `shadowrun_ghidra_conversion_qt_field_icarus_caleb_entry_qa` starts south of
  the ROM rectangle, sets the known access fact and New Game FFFC27/counter
  values, then uses ordinary Up input. It verifies the first page, Caleb's
  continuation, three-choice root, FFFC27 bits 4/7, and all three source-byte
  counters at +2. Debug and Release capture the complete 320x224 pages and
  root under `qt_runtime/field-icarus-caleb-entry-qa/`; matching file hashes
  confirm deterministic host output. I inspected the full Debug and Release
  captures.
- `shadowrun_ghidra_conversion_qt_field_icarus_blocked_entry_qa` uses the same
  physical checkpoint with FFFC1E bit 10 clear. Debug and Release reach the
  bodyguard page through normal movement, verify the clear gate and unchanged
  FFFC27/runner counters, and write complete 320x224 `access-blocked` and
  `actual` captures. I inspected both configurations; their capture hashes
  match.
- The dispatcher selects first-meeting descriptors `0x1056AC` / `0x1056F8`
  and text `0x10863E` / `0x1086C6`, a repeat opening at `0x1056C0` / `0x108886`,
  or the conditional post-run descriptor `0x1059A4` / text `0x105C55`. The
  post-run condition is the separate `FFFBEC` / `FFFC1C` gate at `0x20AFC`;
  its story meaning is not resolved here.
- Caleb's information/work/leave labels resolve to `0x104051`, `0x103A3A`,
  and `0x104FEB`. ROM text for the contacts offer, work introduction, two
  Renraku pages, recommended run, Matrix prerequisite, exit, and transaction
  is imported into `assets/dialogue/script-text.json`. The complete menu
  transitions and run effects remain untraced.
- The default actor list for scene 12 is empty. Caleb's portrait is dialogue
  art and does not establish a standing NPC. The two floor events remain
  visible as unresolved records instead of invented exits.

## Visual verification questions

1. **Does it look correct?** The room annotations follow the extracted
   scene-12 composite: the north-west booth and wall panel, north-east display
   counter, five repeated table/booth fixtures, and five planters have named
   entity definitions. The room pixels remain visible behind the outlines.
2. **Does the look reflect game state?** The booth event links to the
   bodyguard gate and Caleb's first/repeat/post-run dialogue profiles. The map
   actor list is empty, so the viewer does not place Caleb as scenery. The
   guide's rep-6 entry note remains separate from `FFFC1E` bit 10.
3. **Are the visuals correct?** The source composite and priority plane are
   separate entity parts. Table surfaces, booth seating, table lamps, planter
   pots, foliage, and the blue wall rail use separate crops/contours. These
   crops were checked against the extracted scene image; no independent
   Genesis capture of this room was found, so this is not a claim of external
   pixel fidelity.
4. **Is anything animated?** The room fixtures are static map art. Scene 12's
   default actor list has no entries. No movement or animation cadence is
   assigned to Caleb's portrait or the static furniture.
5. **Is there sound?** The map audio tab remains explicit that no scene-specific
   track has been confirmed. This slice did not infer a sound from the room
   appearance; entry/dialogue playback is a separate unresolved audio trace.
6. **Are the other screen elements clean?** The map, priority overlay, ROM
   events, collision layer, actor candidates, and authored entities remain
   separate viewer layers. These annotations add no game buttons or dialogue
   widgets over the room image.
7. **Do multiple frames support the interpretation?** The map pixels and event
   coordinates support the geometry and booth trigger association. There is
   no second independent gameplay frame of this room in the currently reviewed
   evidence, so repeated-frame visual corroboration remains pending.

The first/repeat/post-run conversation entry and work-offer handoff are
connected. The information choice's contacts offer, its 2,000-nuyen purchase
result, work acceptance/run settlement, and the meaning/writers for the entry
and post-run gates still need ROM tracing and outcome tests. The physical
captures are host regressions; no independent Genesis frame of this encounter
has been identified. The two floor records remain unresolved.
