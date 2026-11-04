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

In the headless port replay, the first completed field frame containing slots
3–5 is 8132. Their initial positions match GPGX exactly (`3480,2200`,
`5300,3700`, `6280,1900`), and their `+0x9E` flags match at `20`. The
port's typed `+0x10` service flags are `05` while GPGX has `25`; that bit is
still represented differently by the port's actor lifecycle model. Port first
spawn is 97 frames later than GPGX frame 8035, and later patrol positions also
differ, so this fixture supports the default-population fix but does not
establish exact cadence or movement equivalence. The A800 gate remains closed
in both traces.

`port_route_frames.jsonl.gz` is the headless Qt replay's completed-frame trace
from the same input route. It contains typed port scene/actor state (including
slots, positions, source flags, and conditional-actor status), not emulated RAM.
`port-frame-10001.png` and `port-frame-10050.png` are the port outputs after
Stoker's exit; the later capture visibly shows a pedestrian in the field. The
CTest route case regenerates the keyboard replay from `route.lua`, compares
spawn and gate facts to the independent GPGX trace, and checks that an active
pedestrian is projected into the field viewport after the exit.
