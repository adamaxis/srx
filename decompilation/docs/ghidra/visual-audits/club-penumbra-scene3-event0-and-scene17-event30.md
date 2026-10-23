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
pending list `0x1E628`, loads the Mako introduction descriptor pair
`0xF4CBC`/`0xF4CD0`, and clears D6. The shared exit helper `0x1FED2` sees the
paid marker `FFF0D6=-1` and leaves that clear D6 intact, so finalizer `0x1FD80`
continues through event-table row 0 to field scene 17. The common field-frame
service at `0x02900` calls `0x04388`, which runs `0x043B6`; that function
drains `FFE120` without a scene gate and installs callback `0x53FFC` for this
list.

The entity route opens a named JSON/Lua graph for the doorman prompt, the
Reputation threshold, rejection pages, and 75-nuyen payment. If the separate
Mako story gates pass, the graph shows the two ROM Mako introduction pages,
then stops at an explicit source boundary because native actor allocation and
the actor-list completion callback remain unimplemented. No Qt route QA has
been run for this new graph. The extractor indexes the pending list's actor
rows under Scene 47's activity table, but the traced Club route reaches Scene
17 before the common list consumer drains it. That table grouping does not
assign a Scene-47 transition. See the [field random-event
audit](../field-random-event-selector.md).

## Pending-list actor transitions and result callback

When `0x43B6` consumes `FFE120 == 0x1E628`, it stores callback `0x53FFC` in
`FFE12E`, calls `0x1DBAA`, sets `FFE177=4`, and spawns the four list records.
The scheduler ensures `FFDF84` is active and sets `FFDF86` to `0x78` (or
`0x258` when `FFE162` is nonzero); this counter is separate from the callback's
dispatch condition.

`0xAB02` decrements `FFE177` when an eligible entity completes its behavior
transition with bit 6 set in its `+0x9E` flags. When the count reaches zero,
the source resets entity service records, moves `FFE12E` to `FFDD2C`, calls
`0x1E2EC` to publish it as `FFE104`, and clears `FFE12E`. The field loop then
dispatches the `FFE104` function pointer. This ties `0x53FFC` to the four
actor behavior transitions; its trigger does not test a player kill flag or
wait directly for `FFDF86` to expire. All four extracted rows have the source
`+0x9E=0x40` override: three Strike Team records and one corp guard. The list
does not contain a template named Mako.

The callback adds 2 to `FFFBFA` and clamps it at 8, adds 2 to the three
runner bytes at `FF018D`, `FF028D`, and `FF038D`, sets `FFFC1E` bit 11, clears
`FFFC30` bits 20 and 21 and sets bit 24, then clears `FFFC38` bit 17. It also
increments `FFFBFB` up to 8 through `0x1F1E0`. Finally it runs the shared mode
exit service with resources `0x122000` and descriptor `0xF4CA8` (text
`0xF98FA`, Mako lying unconscious), then returns D6=-1. The ROM callback is
now source-traced; native actor transitions, this delayed story outcome, and
its route QA are still open.

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
queueing list `0x1E628`. The pending-list consumer at `0x43B6` installs
callback `0x53FFC`; after four `FFE177`-counted actor behavior transitions,
it marks Mako defeated and opens descriptor
`0xF4CA8` / text `0xF98FA`. The menu caller is now linked to the Club
Penumbra Scene-3/event-0 finalizer. The finalizer permits row 0 to reach Scene
17, and the next common world-frame service drains the queued list there; the
Scene-47 extraction context is not a destination. Native actor allocation and
the timeout callback remain open. Keep this separate from the traced
scene-17/event-30 generic contact.

## Verification boundary

The existing route test only checks the scene-3/event-0 numeric destination
and the scene-17/event-30 Lua contact binding; it does not cover the doorman
graph added in this continuation. Physical movement into the scene-17
rectangle, the south-edge return, Qt execution of the fee/reputation branches,
pending-list lifecycle and activity placement, and an independent Genesis
capture remain open. The ROM
does not identify the Scene-17 event-30 speaker as Freya; that contact remains
separate from the entry gate.
