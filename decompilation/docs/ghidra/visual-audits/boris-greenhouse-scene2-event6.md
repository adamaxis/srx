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
`0x1F1E0`, then selects two Strike Team actors from `0x1D6FD0`. The Lua graph
now applies that shared typed counter update before retaining a source boundary
for the unconverted actor-list/world-effect/combat handoff. The extracted
world-activity actor catalog has no Scene-2 placement records for list
`0x1D6FD0`, so the later event-list placement data is not being substituted.
The ROM list itself contains `0x001D6AFA` twice and then a null sentinel at
`0x1D6FD8`; template `0x1D6AFA` matches the extracted Strike Team profile in
pending list `0x1E628`. Composition is therefore known, while Scene-2 placement
and combat completion remain open.
Commands `0x2D`-`0x2F` and their item-purchase return paths remain outside this
implementation.

## Implementation and evidence

`conversations/boris_greenhouse.json` owns the currently converted prompts,
payment branches, topic conditions, and `continue_in_place` terminal.
`conditions.json` keeps the signed alternate-result and ordered quest gates as
named expressions. `boris_greenhouse.lua` registers only the named state
mutations for the source commands. The entity link keeps the source table
destination as evidence while priority 10 selects the Lua task. C++ presents
the dialogue and persists FFFC26 at save payload offset `0x22C`. Text and the
profile portrait come from the explicit ROM asset extraction.

The full Lua suite covers signed alternate-result pages, first/repeat greeting,
$100/$500 payment outcomes, ordered topic visibility and effects, and unknown
facts. Debug and Release Qt QA walk ordinary Up into Scene-2 record 87, run the
$100 story branch through its typed same-map return, and verify FFFC26 and
nuyen across save restore. Six complete 320x224 captures per build match by
SHA-256; they are internal regression evidence, not independent Genesis
visual proof. No independent Genesis dialogue frame has been identified. The
route remains partial because command `0x2C`, commands `0x2D`-`0x2F`, and their
follow-on effects are not converted.
