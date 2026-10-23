# Boris Greenhouse: Scene 2 event 6

## ROM route

Scene 2 record 87 is at `0x173110`. Its event code is 6, and the event table's
row 6 contains the byte for Scene 26. Indexed title service 10 at `0x53F8C`
matches Scene 2/event 6 first, loads dialogue profile `0x121F58`, calls the
shared runner at `0x1F246`, and returns D6=-1. The table's Scene-26 value is
therefore suppressed; closing the service returns to Scene 2.

The service dispatches on signed `FFF0D5`, byte `FFFC26`, and story/item flags.
The default path uses text roots at `0xF4C70` and `0xF4C84`. Command `0x25`
checks FFFC26 bit 3; command `0x26` selects the $100 story offer from bit 0;
command `0x27` charges the wallet and sets bit 0. Command `0x29` charges $500
and sets bit 2 before the gated topic list. Michael, David, and Ilene choices
also update the source FFFC1E/FFFC30/FFFC34 facts. The Brother? choice reaches
command `0x2C` at `0x21BE4`: it increments FFFBFB through the capped helper at
`0x1F1E0`, then passes `0x1D6FD0` and `D0=1` to `0x1E1FC`. Raw ROM code at
`0x1E216` sends nonzero D0 through `0x1E40C`, which builds candidates from the
live camera origin, map bounds, and terrain lookup at `0x1E41E`; the zero-D0
branch at `0x1E302` is the separate screen-edge placement path. The list itself
contains `0x001D6AFA` twice and then a null sentinel at `0x1D6FD8`. Template
`0x1D6AFA` matches the extracted Strike Team profile in pending list `0x1E628`.
Command `0x2C` returns prompt group `0xF546C`. Its first text entry `0xF95E2`
contains the Strike Team warning, and its only option uses the shared MORE
caption at `0xFE5BF`. The explicit extractor's record view is for text records,
so the group pointer itself is not a text-record address. The typed host
models the nonzero-D0 map candidates and the separate zero-D0 edge policy. The
two `0x1D6FD0` actor rows are imported from the verified template and resolved
by a named Lua sequence; the field host then attempts the two spawns at open
map-derived candidates before presenting the response page. Combat behavior and the
post-combat return remain open. The later event-list placement coordinates are
unrelated and are not used as substitutes.
The service-sales path continues from the $100 story answer through prompt
groups `0xF52D8` and `0xF52EC`, then the goods prompt at `0xF54FC`. Equipment
prompt `0xF5518` offers the passport and rating-2 passkey; firearm prompt
`0xF5550` offers the Mach 22. Command `0x2D` at `0x21C0C` charges 1,000 nuyen
through helper `0x21A4E`, sets FFFC22 bit 1 (Indian Lands Visa), and returns
the purchase-confirmation/main-choice group at `0xF5534`. Its insufficient
funds page is `0xF5AD8`/`0xF8A88`. Command `0x2E` at `0x21C38` calls
`0x22874`, which scans the active runner's eight item rows from 7 down to 0
and leaves the lowest free slot in D6. A full inventory selects group
`0xF5AE8` and text record `0xF9570`. With room available, the command charges
10,000 through `0x21A4E`, writes item `0x14` (Maglock Passkey) with rating 2,
and returns to `0xF5534`. Its low-funds response reuses `0xF5AD8`/`0xF8A88`.
Command `0x2F` at `0x21C72` uses the same `0x22874` slot scan. A full inventory
returns `0xF5AE8`; otherwise it charges `0x4B0` (1,200 nuyen) through
`0x21A4E`, writes item `0x0A` (Mach 22) in the selected row, clears that row's
`+8` phase byte, and returns to `0xF5534`. Insufficient funds select
`0xF5AD8`/`0xF8A88`.

## Implementation and evidence

`conversations/boris_greenhouse.json` owns the converted prompts, payment,
passport, passkey, and Mach 22 branches, topic conditions, and
`continue_in_place` terminal.
`conditions.json` keeps the signed alternate-result and ordered quest gates as
named expressions. `boris_greenhouse.lua` registers only the named state
mutations for the source commands. The entity link keeps the source table
destination as evidence while priority 10 selects the Lua task. C++ presents
the dialogue and persists FFFC26 at save payload offset `0x22C`. Text and the
profile portrait come from the explicit ROM asset extraction. The speaker
profile binds `david_brother` to the extracted `0xF95E2` response and shared
MORE prompt.

The Boris Lua suite covers signed alternate-result pages, greetings, payment
outcomes, Visa, passkey, and Mach 22 purchases, full-inventory and
insufficient-funds responses, ordered topic visibility and effects, unknown
facts, and both available and unavailable Strike Team counter results. Debug
and Release Qt QA walk ordinary Up into Scene-2 record 87, pay the $100 story
fee, buy the Visa for $1,000, and buy the rating-2 passkey for the exact
remaining $10,000. It checks the first free inventory slot, the purchase
confirmation/main-choice prompt, FFFC26/FFFC22, and the runner item/rating
across save restore. Separate fresh-entry routes confirm that a full inventory
is reported before the empty wallet is checked and that $9,999 leaves both
wallet and inventory unchanged. The dedicated Mach 22 route selects its
firearms menu entry with ordinary input, spends exactly 1,200 nuyen, writes
item `0x0A` into the seeded first free slot, checks phase byte 0, dismisses to
Scene 2, and verifies the purchase in the save snapshot and restored game. Its
`actual.bmp` is a full 320x224 field capture; the purchase confirmation is
captured separately. The Debug and Release Mach 22 captures have identical
SHA-256 hashes. The dialogue route writes 11 full 320x224 purchase and return
frames; each rejection route writes its response and `actual.bmp`. Those
Debug/Release images also match byte-for-byte. These are internal regression
frames, not independent Genesis visual proof. The focused command-`0x2C` route
separately seeds the
paid information byte and known topic facts, selects Information, David, and
Brother? with ordinary buttons, verifies the counter increment and both
spawned actors, captures the response, and dismisses back to Scene 2. The
strike-team route's `actual.bmp`
shows both actors active. No independent Genesis dialogue frame has been
identified. The route remains partial because combat completion, post-combat
return, deeper service outcomes, and independent Genesis dialogue frames
remain open.
