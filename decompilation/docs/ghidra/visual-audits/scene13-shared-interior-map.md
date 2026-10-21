# Scene 13 shared interior map

Status: partial. The map viewer now has named visual entities for the room
fixtures and source-linked Gunderson interaction. Debug and Release physical
input QA verifies the scene-13 south-edge event-0 route into numeric field
scene 0. The work-contract branch remains unresolved.

## Source profile

- Scene 2 event record 85 at `0x1730FC` is event 4; its route-table byte at
  `0x172CE0` selects numeric field scene 13. The separate indexed title
  service may show the Jump House entry page before the map loads.
- Puyallup Scene 1 record 69/event 5 (guide crosswalk: Crime Mall) and record
  72/event 3 (guide crosswalk: Riannon's Magic Shop) both select the same
  numeric scene 13 without an entry dialogue in ordinary-input route QA.
  FFFC1E bit 19 is tested after scene preparation and selects script 3 with
  `lone11_k0a_item03` at `(4352,768)`; the flag does not gate either entrance.
  This route evidence does not establish that the shared room is a named shop
  interior or that the Lua service-title key opens a shop catalog.
- Puyallup record 74/event 6 (guide crosswalk: Dr. Bob's Quickstitch Clinic)
  also selects Scene 13 with the same zero-fund/no-dialogue behavior and bit-19
  actor gate. Its dedicated Debug/Release route QA verifies the source
  rectangle and transition; it does not establish a clinic menu or treatment
  interaction.
- Puyallup records 76/77/event 9 (guide crosswalk: the Ork Gang Hideout) use
  Scene 13 as the ordinary event-table route. ROM `0x1F086` intercepts an
  active gang-contract target matching area 1/event 9, presents the entry
  passage from `0xFE0DE`, and selects the Scene-15 actor group instead. A
  different-area active target and no active run both follow Scene 13. The
  Debug/Release physical QA checks the matching branch at record 76 and the
  mismatched branch at adjacent record 77; it does not complete the gang fight
  or reward path.
- Penumbra Scene 3 record 100/event 4 (guide and contract crosswalk:
  Eye-Fivers) also uses Scene 13 as its generic route. An exact active
  area-3/event-4 gang target selects the passage and Scene-15 actor group at
  `0x1F086`; a different active target and no active contract both follow row
  4 to Scene 13. Debug/Release ordinary-input QA checks all three states.
- Scene 13 has no default map actors. The Gunderson portrait selected by event
  30 is dialogue art, not a resident sprite.
- Scene 13 records 11 and 12 (`0x1B3CF8`, `0x1B3D02`) are adjacent event-30
  cells at map pixels `(0,32)` and `(32,32)`. ROM `0xB7B8` dispatches them to
  `0x20990`, which selects dialogue row 0, profile `0x121E80`, and root
  descriptor `0x105114`.
- The root menu labels resolve to `0x104051` (information), `0x103A3A`
  (work), and `0x104FEB` (leave). The Lua dialogue profile exposes the
  information/question/contact branches, including the 1,000-nuyen Decker
  contact check. The work contract is not connected yet.
- Scene 13 records 9 and 10 (`0x1B3CE4`, `0x1B3CEE`) are event-0 cells at
  `(64,224)` and `(96,224)`, at the south edge of the map. The profile's event
  table pointer is null; the generic selector at `0x167AE` reads ROM byte 0,
  selecting numeric field scene 0. Lua records this as a static area-event
  override. Debug and Release ordinary-Down QA reaches scene 0 from the Scene
  13 profile spawn and saves full 320x224 frames; matching captures have the
  same SHA-256. The trace does not identify scene 0 as the prior caller map or
  establish a door-relative spawn/facing rule.
- Other district maps also have ROM routes to numeric scene 13. The viewer
  calls it a shared interior map and shows incoming routes individually; the
  Jump House name is a route cross-reference, not a universal scene identity.

## Visual verification questions

1. **Does it look correct?** The map includes separately named regions for the
   northwest booth, north counter, green wall displays, the two lower booths,
   west wall-side table, east inset, and barrel stack. No overlay replaces the
   extracted map pixels.
2. **Does the look reflect behavior?** Event 30 is attached to the northwest
   booth, where the source branch selects Gunderson. Scene 13 has no default
   actor records, so no NPC body is placed in the room. The event-0 cells select
   field scene 0 through the null-table fallback; they have no traced dialogue.
   The runner enters at Scene 13's source profile position and walks south to
   those cells during physical QA.
3. **Are the visuals correct?** The room composite and priority plane are
   treated separately. Counter rail, table lamps, and the inset's bright edge
   are priority parts; no single rectangle is used to label the whole floor.
   The map art is the ROM extraction. No new actor or building art is invented.
4. **Is anything animated?** The visible room fixtures are static. The
   conversation uses the shared dialogue portrait and pages; its exact idle
   animation/audio cadence remains a separate source trace.
5. **Is there sound?** No Scene-13 map track or event-specific sound is
   identified by this slice. The Map audio tab reports that as unresolved; it
   does not call the scene silent.
6. **Are other screen elements clean?** The map image, priority overlay,
   default-spawn markers, actor candidates, ROM triggers, and authored objects
   remain separate viewer layers. This slice adds no game UI over the room.
7. **Do multiple frames support the interpretation?** The supplied longplay
   contains multiple Jump House entry and Gunderson dialogue frames documented
   in `jump-house-entry-and-gunderson.md`. Those frames support the dialogue
   portrait/panel behavior, not the map's full object contours or every
   incoming route into shared scene 13.

The next pass should complete the Gunderson work-contract state flow and
profile the remaining shared map destinations before assigning them building
names.
