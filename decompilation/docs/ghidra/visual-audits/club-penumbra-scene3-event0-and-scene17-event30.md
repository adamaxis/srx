# Club Penumbra scene-3 entry and scene-17 event-30 contact

## Exterior route

Scene 3 collision record 101 at ROM `0x18ABC8` covers map cell `(56,28)` and
publishes event 0. Scene 3's event table at `0x18A6FC` has numeric destination
`0x11`, field scene 17, at row 0. The event record and contract-location index
0 cross-reference the site name Club Penumbra; the route byte itself names
only scene 17.

The indexed title-service dispatcher at `0x53A78` calls the 23 entries in the
ROM table at `0x15FD5C` before generic exit routing. None of the nonempty
handlers matches scene 3/event 0. The entry selector `0x20634` and exit
selectors `0x1F65A`, `0x1F830`, and `0x1FA78` have no scene-3/event-0 branch;
`0x1FD80` only reaches the generic scene-3 finalizer, with no event-specific
reputation or nuyen check. The ordinary event-table path therefore selects
scene 17 without a ROM entry gate or exterior dialogue. The external map
guide's reputation-4, 75-nuyen entry requirement is not supported by this ROM
path.

## Scene-17 contact

Scene 17 collision record 10 at `0x1D63B4` has raw value `0x019E`/event 30 and
covers map pixels `(0,32)..(31,63)`. Event 30 bypasses the scene event table and
enters `0x20990`. Scene 17 (`0x11`) matches none of that handler's special
scene cases, so it reaches the default branch at `0x20C78`. That branch loads
portrait profile `0x121E5C` and prompt record `0xF3FAC`; text record `0xF3FBC`
is “An old man flashes his gums at you.” The prompt has no choice pointer, so
this is a one-page contact, not a gated entry menu. The source does not name
the speaker Freya. The scene-17 ROM profile's default actor list has zero
records; this does not rule out conditional or event-created actor behavior,
but it supplies no default Freya map actor.

The generic location-dialogue task now binds this event through the Lua entity
link at `gameplay/world/entities/club_penumbra_unidentified_contact/links.tsv`.
The scene-17 area record places the contact at the exact ROM rectangle. It
uses the shared old-man text profile and has no `route_conditions`, matching
the handler's ungated branch. The exterior route remains a separate link on
the Penumbra site entity and continues to validate destination 17 against the
active ROM event-table row.

## Verification boundary

The runtime route test checks both the scene-3/event-0 numeric transition and
the scene-17/event-30 Lua contact binding. Physical movement into the scene-17
rectangle, the scene-17 south-edge return, and an independent Genesis capture
of the contact remain unverified. The ROM route establishes no Freya dialogue,
fee, or reputation threshold here, so those guide details remain unresolved
content rather than invented runtime behavior.
