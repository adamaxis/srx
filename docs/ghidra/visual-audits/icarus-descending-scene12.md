# Icarus Descending (field scene 12)

Status: partial. The map viewer now has source-linked room fixtures and the
Caleb Brightmore text/portrait profile. The native game route does not yet run
the complete Icarus conversation, contact purchase, or job branches.

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
- The dispatcher selects first-meeting descriptors `0x1056AC` / `0x1056F8`
  and text `0x10863E` / `0x1086C6`, a repeat opening at `0x1056C0` / `0x108886`,
  or the conditional post-run descriptor `0x1059A4` / text `0x105C55`. The
  post-run condition is the separate `FFFBEC` / `FFFC1C` gate at `0x20AFC`;
  its story meaning is not resolved here.
- Caleb's information/work/leave labels resolve to `0x104051`, `0x103A3A`,
  and `0x104FEB`. ROM text for the contacts offer, work introduction, two
  Renraku pages, recommended run, Matrix prerequisite, exit, and transaction
  is imported into `assets/dialogue/script-text.lua`. The complete menu
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

The next source pass should trace the Caleb information/work choice commands,
contact purchase outcomes, and the meaning/writers of the two state gates
before connecting those branches to the native runtime.
