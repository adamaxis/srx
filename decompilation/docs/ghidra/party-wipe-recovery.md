# Party wipe run failure and wake route

Status: partial. The ROM control flow, run-failure text, recovery values, and
the two destination arrival records are traced. Lua owns the active-run failure
decision, post-wipe runner selection, and wake destination. The C++ field host
applies typed party/field writes, refreshes the player HUD state, presents the
staged dialogues, and runs the existing scene fade. The seven-record world
actor cleanup and the world-actor part of 0x391A remain for the combat branch
[C].

## Entry and failure branch

ROM `0xABB6` arms FFE166 with `0x3C` frames when the leader-down record has bit
6 set and all recruited runners are inactive or dead. The field loop decrements
that timer at `0x1DDC6`; it enters `0xC64C` when it reaches zero [C].

`0xC64C` first clears FFE186, FFE182, and FFF10C, then dispatches the low byte
of FFFBED with status in FFFC1C [C]:

| Run type | Gate | Source behavior |
| --- | --- | --- |
| `5` employee recruitment | FFFC1C bit 0 set and bit 7 clear | Clear FFE183, call `0x1DED8`, show node `0xF3D5C`, reset through `0x1E68C`, then call `0x1F246` with profile `0x1220B4`. |
| `1` bodyguard escort | FFFC1C bit 0 set | Call `0x1DED8`, show node `0xF359E`, then the shared reset/dialogue path. This branch does not execute the FFE183 clear at `0xC684`. |
| `2` package delivery | FFFC1C bit 0 set | Clear FFFC22 bit 9, show node `0xF2510`, then reset through `0x1E68C` and call `0x1F246`. |
| `4` package recovery | FFFC1C bit 0 set | Branch to the type-2 path at `0xC6E8`. |
| Other types, no active bit, or type 5 with bit 7 set | — | Continue directly to the common recovery at `0xC716`. |

`0x1DED8` clears FFFC3C, then scans seven world-actor records from FF0400
through FF0A00, stride 0x100, calling `0x1E514` for records whose +0xB0 byte
has bit 7 set [C]. The run-failure result requests the field-side FFFC3C clear;
C++ applies it to its typed player-spawn marker. The record scan and its
`0x1E514` actor mutations remain an explicit combat-branch boundary [C].
`0x0A72` clears nonzero FFE18F and sets FFFC1C bit 7; `0x1EEF0` also checks
that bit, and `0xC678` uses it as the type-5 wipe guard [C]. FFFC1C bit 7 has
other source writers, including the type-0 Ghoul Bounty ready gate at
`0x1F1FC`; those unrelated bits are not represented by the active-offer
registry [C/H]. The C++ host passes its typed scene-reset bit to Lua, while the
Lua run registry supplies the active run and type as a host model [H].
`0x1E68C` writes FFFBEC = FFFF and clears FFFC1C [C]; the native path drops the
active offer and clears its tracked bit-7 mirror there.

The three failure nodes use their first pointer as a one-page text record. At
each node the second longword is FFFFFFFF, so the dialogue routine returns
after that page without using the later option columns [C]. Their first text
streams are now named in `assets/dialogue/script-text.json`, with speaker events
in `gameplay/dialogue/speakers.json`.

## Common recovery and destination

After the optional failure page, `0xC716` clears FF024C and FF034C, calls
`0xC802`, revives the leader, clears DEAD/DOWN, sets ACTIVE, and floors the
leader's physical and mental meters at 6 and 8. It deducts one eighth of the
wallet, capped at 5000 nuyen. The player-side model also zeros the two
hired-runner +0x4C delay meters [C].

The wipe then calls `0x38FA` and the full `0x391A` route before selecting the
wake destination [C]. Lua now performs the `0x391A` type-2-wrap runner scan,
selection-marker updates, and FFF09D suppression gate from typed player-record
facts. C++ applies the selected slot and marker writes, initializes the typed
FFDFDA/FFDFDC fade marker through `0x1E540` when selection changes, requests
event `0x25`, and rebuilds all three HUD slot states [C]. Refreshing the host
player projection, using named HUD/field assets in place of the source
`0x33AA` uploads and `0x3548` tile-panel drawing, and keeping animation on the
existing host heartbeat are host substitutions [H].

The next `0x391A` call, `0x1DC4C`, scans and updates world-actor records. That
call remains a combat-branch boundary [C]. Independent Genesis evidence for
the post-wipe recruited-runner presentation remains open [H].

The wake choice reads FFFC1E bit 0. The paid motel branch at `0x2A32` uses node
`0xC25A0`, destination ID 0, and continuation record `0x2AEA`; the other branch
at `0x2A44` uses node `0xC25BC`, destination ID 2, and record `0x2B24` [C].
`0x2AEA` contains raw coordinates `(0x6260, 0x2D40)`, which the source aligns
to `(0x6200, 0x2D00)`. `0x2B24` contains `(0x66F0, 0x2310)`, aligned to
`(0x6680, 0x2300)` by the `0x2B78` continuation [C].

After the wake dialogue closes, `0x136DE` installs the destination ID and the
scene loader publishes it through FFFBF8 [C]. Lua selects the named
`E.PartyWipeDestination` and `E.PartyWipeArrival` data; C++ passes those values
through its existing dialogue scene fade route. Explicit spawn data also
forces a real scene entry when DocWagon returns to the already loaded scene.

The source restore path at `0x594E0` reads bytes FFF5BE through FFF805 (end
pointer FFF806) for the saved field payload and does not restore FFE166, so the
countdown does not survive a ROM save load [C]. Whether the save writer
separately copies the countdown is not traced [H]. The native game snapshot
also leaves this transient out.

## Checks

`party_down_tests.lua` covers types 1, 2, 4, and 5; type 5's bit-7 guard; the
active-bit and no-run cases; run-registry reset; and both destination/arrival
choices. `shadowrun_ghidra_conversion_qt_field_party_wipe_qa` runs the common
recovery twice and checks DocWagon scene 0 at `(0x6200, 0x2D00)` and Little
Chiba scene 2 at `(0x6680, 0x2300)`. It writes `docwagon.bmp` and
`actual.bmp`. These are native regression dumps; no independent Genesis frame
has been claimed.
