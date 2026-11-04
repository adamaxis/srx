# GPGX party down route fixture

This fixture starts from a cold boot and reuses
`../gpgx_stoker_exit_pedestrians/route.lua` for New Game, Samurai, Stoker's,
and the outdoor route. It then presses B for one frame at route frame 12000
and holds A starting at frame 12030 to enter the nearby real encounter. There
are no RAM writes, savestate loads, or debug teleports.

`gpgx_party_down_frames.tsv` records player slots 0–2 around the first zero
meter and recovery. At frame 21092 the leader's mental meter reaches 0. By
frame 21104 the source down bit is set and FFE166 has 0x003B remaining; the
countdown reaches zero at frame 21163. At frame 21181 the leader is active
again with physical 9 and mental 8. Screenshots in this directory are the
corresponding independent GPGX video frames.

ROM path: `0xA8EA` checks +0x9C/+0x4C and dispatches `0xB330` on zero;
`0xABB6..0xABF2` tests party availability and arms FFE166; `0x1DDC6` advances
the countdown and dispatches `0xC64C`; `0xC716` restores the leader with
source meter floors. These values and captures come from the US ROM running
under Genesis Plus GX, not from the port.
