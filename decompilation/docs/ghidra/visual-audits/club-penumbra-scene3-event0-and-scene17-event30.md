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
selectors `0x1F65A`, `0x1F830`, and `0x1FA78` also have no scene-3/event-0
branch. The later finalizer `0x1FD80`, however, does: for
`FFFBF8=3` and `FFFBF9=0`, it loads doorman menu `0x110C7A` and runs the
shared prompt path. That menu's text at `0x1148F8` names Club Penumbra and
asks the player's business. Choosing “I'd like to go in” executes command
`0x5F`; handler `0x225B6` checks active-runner Reputation at `FF018B`,
returning rejection node `0x11132E` below 4 and fee node `0x111316` at 4 or
higher. The fee node asks for 75 nuyen at `0x114A88`; command `0x60` at
`0x225D2` debits that fee. When its additional story gates pass, it queues
pending list `0x1E628`.

The event-table row still names scene 17 as the ordinary destination, but the
doorman conversation now sits between the exterior event and that transition.
The entity route opens a named JSON/Lua graph for the doorman prompt, the
Reputation threshold, rejection pages, and 75-nuyen payment. If the separate
Mako story gates pass, that graph stops at an explicit source boundary because
the pending actor-list lifecycle is still unimplemented. No Qt route QA has
been run for this new graph. The extractor indexes the pending list's actor
rows under Scene 47's activity table, but the common list consumer does not
itself establish that scene as a destination. The path from the Club prompt
to the list's activity placement remains unresolved. See the [field
random-event audit](../field-random-event-selector.md).

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
link at `gameplay/world/entities/club_penumbra_unidentified_contact/interactions.json`.
The scene-17 area record places the contact at the exact ROM rectangle. It
uses the shared old-man text profile and has no route condition reference in
`gameplay/dialogue/location_conditions.json`, matching the handler's ungated
branch. The exterior route remains a separate link on the Penumbra site
entity and continues to validate destination 17 against the active ROM
event-table row.

The fee node `0x111316` contains command `0x60` in its first choice slot at
`0x111326`; adjacent code at `0x225CA` returns this node in A0. Its first
choice label is `0x103A7A` (“Easy money, chummer. I'll pay”), and its second is
`0x104FEB` (“I'll talk to you later, chum”). On the paid path, the handler
checks FFFC1E bits 10/11 and either FFFC30 bit 21 or FFFC38 bit 3 before
queueing list `0x1E628`. The pending-list consumer at `0x43B6` installs timeout
callback `0x53FFC`; that completion marks Mako defeated and opens descriptor
`0xF4CA8` / text `0xF98FA`. The menu caller is now linked to the Club
Penumbra Scene-3/event-0 finalizer. The extracted actor rows are indexed under
Scene 47, while exact activity placement, callback timing, and runtime flow
remain open. Keep this separate from the traced scene-17/event-30 generic
contact.

## Verification boundary

The existing route test only checks the scene-3/event-0 numeric destination
and the scene-17/event-30 Lua contact binding; it does not cover the doorman
graph added in this continuation. Physical movement into the scene-17
rectangle, the south-edge return, Qt execution of the fee/reputation branches,
pending-list lifecycle and activity placement, and an independent Genesis
capture remain open. The ROM
does not identify the Scene-17 event-30 speaker as Freya; that contact remains
separate from the entry gate.
