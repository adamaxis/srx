# GPGX FF1700 obstruction-grid captures

Both files are direct 4096-byte reads of GPGX work RAM `FF1700..FF26FF`; the fixture values do not come from the port.
The Lua rasterizer used to compare scene rectangles with these captures lives under `tests_lua/support/` and is test-only. Gameplay collision remains in the native field collision service; the runtime does not allocate or query an FF1700-shaped Lua table.

| Fixture | GPGX savestate | Continuation input/frame | Scene | SHA-256 |
|---|---|---|---:|---|
| `ff1700_hw2_f64.bin` | `/workspace/shadowrun/emu/doors/hw2.state` (state SHA-256 `b9bc8cfbe6341aa93be083c82706977c374e302e4ef01fee42760842eb245019`) | #49 Step 1 script: frame 0 neutral, 1–30 Right, 31–70 Up, then neutral; frame 64 | 2 (`FFDDC4=0002`) | `de2ca8cc74344ebb567161731bba2c0867e03714c3d9592433cabe2ee4d33c7b` |
| `ff1700_10080_f264.bin` | `/workspace/shadowrun/emu/doors/playtest46_gpgx_hit_10080.state` (state SHA-256 `3a177c2af4cb78803679be3b8c4deecd76b663786ddea758fcb9d77b74f5ef60`) | `/workspace/shadowrun/out/p49b/repro.txt`: frame 0 neutral, 1–30 Right, 31–70 Up, then neutral; continuation frame 264 | 2 (`FFDDC4=0002`) | `de2ca8cc74344ebb567161731bba2c0867e03714c3d9592433cabe2ee4d33c7b` |

The first capture used the existing GPGX core (SHA-256 `1befd713a98a8d539dfc51ba1263a5c4275ee17500c7dc3a4aa4435ccf854cf1`). The continuation capture used the instrumented GPGX core named in `p49b/repro.txt` (SHA-256 `794f715db22531fed3bb8634e95c59a32fd5f18c15a2419faadafef8f1b3e73d`). Both sampled states have `FFDF8A=0`; the resulting grids are byte-identical.
