# Playtest #34c pedestrian route evidence

`route.lua` is controller input from a cold boot: Start, New Game, Samurai,
Stoker's dialogue, exit, then ordinary walking. It contains no RAM writes,
savestate loads, debug teleport, or scene-load shortcut.

`gpgx_route_frames.tsv` records every zero-based GPGX frame from the verified US
ROM (`rom.sha256`); slot 3–5 record positions, flags, continuations, and route
pointers are in the final column. The A800 selector inputs are recorded in
separate columns. GPGX first enters Scene 2 at frame 8028 and activates default
pedestrian slots 3–5 at frame 8035. At frame 10000, slot 3 is visible on its
Scene-2 patrol route. Numbered `gpgx-frame-*.png` captures show the source route.
The final GPGX savestate is `/workspace/shadowrun/emu/doors/bug34c_stoker_exit_route.state`.

GPGX activates Scene-2 slots 3, 4, and 5 at frame 8035. Their positions are
unchanged at frame 8134; slot 3 first moves at frame 8138. The ROM source order
is `0x0281E -> 0x02900 -> 0x04388 -> 0x043B6`; the refresh flag is consumed by
`0x0EE10`, before the 31 warmup services at `0x08C0`. The actor path at
`0xAC84 -> 0x13904 -> 0xAC9E -> 0xAE5E -> 0xB45C` keeps these active records on
the idle collision continuation while `FFE18A` is set. The port labels its
initial `field_ready` presentation phase at a different point in startup, so
its application frame counter is not a direct offset from GPGX's `retro_run`
counter. The route CTest checks publication in the first Scene-2 field row,
the source positions and `+0x9E` flags, and the first movement vector. The
port's typed `+0x10` service flags are `05` while GPGX has `25`; that bit
remains a separate actor-lifecycle question, and the test does not establish
exact cadence or full patrol-path equivalence. The A800 gate remains closed
in both traces.

`port_route_frames.jsonl.gz` is the headless Qt replay's completed-frame trace
from the same input route. It contains typed port scene/actor state (including
slots, positions, source flags, and conditional-actor status), not emulated RAM.
`port-frame-10001.png` and `port-frame-10050.png` are the port outputs after
Stoker's exit; the later capture visibly shows a pedestrian in the field. The
CTest route case regenerates the keyboard replay from `route.lua`, compares
spawn and gate facts to the independent GPGX trace, and checks that an active
pedestrian is projected into the field viewport after the exit.
