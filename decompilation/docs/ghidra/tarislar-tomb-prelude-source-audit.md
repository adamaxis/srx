# Tarislar Tomb pre-credit presentation source audit

Date: 2026-10-23

This note records the primary-ROM timing and resource path inside the Scene-55
credits callback. It is an implementation input for native Lua state and
named host assets; it is not a plan to reproduce the ROM's sprite-record
builder, VDP, or register state at runtime.

## Entry and clock

After the Licourtrix prompt at `0x5BAC4` returns, the callback saves
`FFF9F8`, sets its temporary value to `0x8F`, clears the scene planes, and
prepares resource descriptor `0x5C966` twice through `0x1406`. It then stores
`FFDF88` in `FFF11E` at
`0x5BB14`. This is the prelude's local time origin. The loop seeds D7 with
`0x03E8`; each pass waits for a VBlank and services controller/input before
calling `0x5BC70`, `0x5C230`, `0x5BD2C`, and `0x5BBA6`. There is no input exit
branch in the loop. Standard `DBRA` semantics run the body for D7 values 1000
through 0, or 1001 passes, before the five credits panels begin at `0x5C3B4`.

The callback restores the saved `FFF9F8` value after the credits panel call.
The host route should keep this as one callback-owned cinematic session; it
should not add a second mutable copy of the ROM work area.

## Source-timed layers

The elapsed values below are `FFDF88 - FFF11E` and are counted in source
service frames. Addresses in the last column are sprite-list data consumed by
the ROM's shared sprite-record builder at `0x1378`; they are provenance for
the extracted host art, not runtime pointers.

| Source time | ROM path | Recovered behavior and resources |
| --- | --- | --- |
| `0..0x6D` | `0x5BD6A` | Draw list `0x1EF988` at base `(0xF0,0x117)` with a small random offset. |
| `0x6E..0xC7` | `0x5BD6A` | Switch to list `0x1EFAAA` at fixed-point position `FFF1B0/FFF1B2`; four of the five `(vx,vy,wait)` rows at `0x5BE62` update the position before the routine stops at `0xC8`. |
| `0..0x351` | `0x5BEA2` | Draw list `0x1EFB5C` at `(0x188, FFF1B8 >> 6)`. The initial y value is `0x44C0`; it moves upward by `0x33` per source frame from `0x190` through `0x1F3`, then holds. |
| `<0xFA` and `0x26C..0x2BB` | `0x5BEA2` | Animate two list families selected from pointer tables `0x5C016` and `0x5C01E`. The first changes every 10 frames; the second every 5. Their lists are `0x1EFB9E`, `0x1EFBB8`, `0x1EFBEC`, `0x1EFBFE`, `0x1EFC10`, and `0x1EFC22`. |
| `0x190..0x351` | `0x5C02E` | Draw list `0x1EF936` at base x `0x1BF` and y `(FFF1B8 >> 6) + 0x4D`. At `0x190`, `0x5C744` loads stream `0x1EFDCC` at tile `0x80`; this list does not use the `0x1F002E` pattern bank. |
| `0xFA..0x26B` and `0x2BC..0x351` | `0x5BEA2` | Draw list `0x1EFBCA` at `(0x188, FFF1B8 >> 6)`. |
| `<0x17C` | `0x5C0A6` | Two fixed-point motion paths draw lists `0x1EFC66` and `0x1EFC34`; their velocity/deadline rows begin at `0x5C1FA` and `0x5C1D0`. Both lists stop after elapsed frame `0x17B`. |
| Timed restarts | `0x5C230`, `0x5C336` | Start six ordered sprite-list phases from table `0x5C39C`: `0x1EFCA8`, `0x1EFCB2`, `0x1EFCCC`, `0x1EFD0E`, `0x1EFD48`, `0x1EFD6A`. A selected phase advances every 8 frames. The initial base is `(0x106,0x10B)`; exact later restart times and bases are listed below. |

The ROM branch bounds make two non-contiguous animation windows. At `0x5BEF6`,
`0x5BEFA`, and `0x5BF06`, the `0x1EFBCA` list is selected for elapsed
`0xFA..0x26B` and `0x2BC..0x351`; the two animated pointer families run at
`0..0xF9` and `0x26C..0x2BB`. `0x5BE80` initializes the shared y position to
`0x44C0` and both family indices/deadlines to zero. The first family contains
two lists and wraps at index 2, while the second contains four and wraps at
index 4. On a frame where both deadlines are reached, the first family updates
before the second; the second family anchors at x `0x14A +
((first_family_index ^ 1) << 2)` and y `(FFF1B8 >> 6) - 4` after that update.
The sprite-service call
order at `0x5BD2C` is `0x5BEA2`, `0x5C02E`, `0x5C336`, `0x5BD6A`, and
`0x5C0A6`; preserve this order when composing the named host layers.

The `0x5BD6A` early actor starts with list `0x1EF988` at `(0xF0,0x117)` and
calls `0xD3A` with bound `0x28` on every active frame. Only results 0 through
3 shift both coordinates by that result. At elapsed `0x6E`, it switches to
list `0x1EFAAA`, initializes fixed-point position `(0x40C0,0x3C00)`, and
begins reading five six-byte records from `0x5BE62` as `(vx,vy,wait)`:
`(0,0,0x1E)`, `(-0x33,0x33,0x0A)`, `(-0x66,0x33,0x0A)`, and
`(-0x33,0x33,0x28)`. The fifth all-zero record is not reached because the
routine returns at elapsed `0xC8`. Position is advanced before drawing and
converted from six-fraction-bit units by shifting right six.

The two paths in `0x5C0A6` also read `(vx,vy,wait)` records, add the selected
velocity on the same frame as the record change, and draw after converting
their six-fraction-bit positions to pixels. They start from `(0x24C0,0x37C0)`.
The `0x1EFC66` path reads `0x5C1FA`: `(0,0,0xA0)`,
`(0x33,0x33,0x0A)`, `(0x33,0x66,0x0A)`, `(0x66,0x66,0x0A)`,
`(0x99,0x99,0x0A)`, `(0x66,0x99,0x0A)`, `(0,0,0x64)`,
`(0,0x80,0x46)`, then an unused zero row. The `0x1EFC34` path reads
`0x5C1D0`: `(0,0,0xAA)`, `(0x33,0x33,0x14)`, `(0x33,0x66,0x0A)`,
`(0x99,0xCC,0x0A)`, `(0,0,0x64)`, `(0,0x80,0x46)`, then two unused
rows. The shared elapsed-`0x17C` cutoff occurs before either path can load
the trailing row.

The six-list phase starts at elapsed `0`, then `0x50`, `0x6E`, `0xA0`, `0xD2`,
`0x26C`, `0x27B`, `0x28A`, and `0x2A3`. The corresponding bases from the
`0x5C230` comparisons are `(0x0D3,0x11C)`, `(0x114,0x101)`, `(0x100,0x150)`,
`(0x0DD,0x133)`, `(0x0A3,0x0FA)`, `(0x0D5,0x0CF)`, `(0x107,0x0B0)`, and
`(0x179,0x0FF)`. The first base is set separately by `0x5C2FE`.

`0x5BC70` also changes the active plane scroll. Descriptor `0x5C966` initializes
horizontal and vertical scroll words at offsets `+8..+0xE` to zero. Between
elapsed `0x64` and `0xA9`, it calls `0xD3A` with bound `0x20` each pass; results
below four replace Plane-A horizontal scroll `FFDD08` with the negative result.
From `0x136` through `0x18F`, while Plane-B vertical scroll `FFDD0E` remains
above signed `-0x90`, each pass subtracts two from both planes' vertical scroll
words (`FFDD0A` and `FFDD0E`). Starting from zero, this reaches `-0x90` after
72 updates, on elapsed `0x17D`; later passes in that interval leave it there.
At `0x190`, `0x5C744` clears both planes' horizontal and vertical scroll words
before installing the skyline. These source scroll changes are absent from
the static importer previews and need typed runtime state before the sequence
can be called reconstructed.

The same routine changes resources at `0x190` and `0x26C`, then calls the
Plane-B clear/fill helper `0x102A` at `0x30C`. This leaves the Plane-A reveal
map in place. After the `0x26C` resource change, `0x5C88C` is called on
even elapsed frames from `0x26E` through
`0x2BA`. It writes one tile in each of 19 rows per update. Its destination
column advances from x=1 through x=38; the source stride is 38 words because
the row writer post-increments one word and then skips 37 more. The counter
guard rejects its call at `0x2BA`, so the 38 visible writes end at `0x2B8`.
This reveals a 38x19 map from `0x1F6532` across Plane A at (1,2), over the
Plane-B scene installed at `0x190`. The separate `0x5BBA6` schedule calls
`0x530DE` with D0 values `0x35` or `0x2C` at exact elapsed values. These paths
are part of the active presentation, so replacing them with a fixed host delay
would omit source-visible state changes.

The `0x5BBA6` sound schedule dispatches `0x35` at elapsed
`0x01,0x1C,0x30,0x45,0x58,0x6C,0x85,0x95,0xA8,0xBC,0xD0,0xE0,0x26F,0x280,0x294,0x2A8`.
It dispatches `0x2C` at `0xA0,0xB0,0xC4,0xF0`. The later `0x2C` comparison
for `0xE0` is unreachable because the earlier `0x35` comparison handles that
elapsed value first.

## Background stages and palette writes

The descriptor at `0x5C966` points to VDP setup `0x5C940`, the four-line
initial palette at `0x5C9D2`, and two tagged `0x1728` layouts. Pattern stream
`0x1EE2AA` is uploaded at VRAM byte address `$4420` (tile `$221`); the tagged
pointer `0x1F002F` selects stream data at `0x1F002E`, uploaded at `$8A80`
(tile `$454`). The Plane-A map `0x1EF775` occupies 16 columns by 14 rows from
row 14 and sets priority. The Plane-B map `0x1EF7FB` occupies 40 columns by
18 rows from row 10. Both maps use palette line 0. The two stream terminators
leave the final cleared cells blank; the offline composition retains those
cells as tile zero.

At elapsed `0x190`, `0x5C744` clears both planes and loads pattern streams
`0x1EFDCC` at tile `$80` and `0x1E6956` at tile `$221`. It first copies fifteen
zero words from `0x5CA52` to palette-line-3 entries 1..15, uploads that palette
change, then draws the 40x15 Plane-B layout from `0x1E8802` at byte address
`$2680` (screen tile row 13) with palette-line-3 attributes. It restores
entries 1..15 from `0x5C9B4` and requests another palette upload. The restored
palette is the steady background state; the zero write is a timed black
transition and is not baked into that static image.

At elapsed `0x26C`, `0x5C842` loads `0x1F310E` at `$BAC0` (tile `$5D6`) and
copies fifteen palette-line-0 colors from `0x5CA74` to CRAM entries 1..15.
The source map at `0x1F6532` is 38 words wide by 19 rows. `0x5C88C` exposes
one column every two service frames, producing a complete 304x152 host layer
after 38 writes. The focused `--scene-55-tomb-prelude-backgrounds` importer
emits four temporary inspection images: the initial two-plane scene, the
steady elapsed-`0x190` skyline, the transparent elapsed-`0x26C` foreground,
and their full-screen composite. The explicit `--all-assets` import now emits
three named host layers under `assets/cinematics/tarislar-tomb/`: the initial
background, the city background, and a cropped 296x152 transparent logo
overlay with source origin `(13,16)`. It also emits 19 cropped sprite-list
images from the timed groups, with each list's source anchor retained as a
relative asset origin and its selected palette resolved at import. The
combined full-screen composite stays an inspection image. These assets come
from source resources and are not independent Genesis captures.

## Extraction boundary

The descriptor's second compressed pattern stream is `0x1F002E`, loaded at
tile `0x454`; it supplies the sprite lists from `0x1EF988` through
`0x1EFD6A` except `0x1EF936`. That list uses the time-`0x190` stream at
`0x1EFDCC`, loaded at tile `0x80`. The offline importer now has a focused
`--scene-55-tomb-prelude-sprite-lists` mode that expands all 19 known lists
into centered inspection images. The legacy `--scene-5bd2c-sprite-lists` flag
still emits its original first two images. The inspection images do not carry
the source timeline, background/layout stages, or all time-dependent palette
changes; they are not runtime assets or visual goldens.

The presentation remains a source boundary after the Licourtrix response.
The stage maps, pattern banks, timed palette writes, and sprite-list resources
are now traced; the explicit importer emits named background/overlay and
sprite assets plus separate inspection previews. Before connecting the
sequence, the remaining work is to represent the timed actor/effect states
in Lua, consume these layers through a typed presenter, and connect the
sequence back to the five credits panels. The existing direct credits-panel
QA does not defend this live callback path.

## Source references

- US ROM: `0x5BAC4`, `0x5BB14`, `0x5BB58`, `0x5BB84`, `0x5BC70`, `0x5BBA6`,
  `0x5BD6A`, `0x5BE62`, `0x5BE80`, `0x5BEA2`, `0x5C016`, `0x5C01E`,
  `0x5C02E`, `0x5C06C`, `0x5C0A6`, `0x5C1D0`, `0x5C1FA`, `0x5C230`,
  `0x5C2FE`, `0x5C336`, `0x5C39C`, `0x5C744`, `0x5C842`, `0x5C88C`,
  `0x5C966`, `0x1EFDCC`, and `0x1F002E`.
- Ghidra cross-check: `fn_00001368.cpp`, `fn_00001406.cpp`,
  `fn_00001530.cpp`, `fn_00001728.cpp`, and `fn_00001378.cpp`; the primary
  ROM instructions at `0x5C744`, `0x5C842`, and `0x5C88C` define the timed
  plane and palette updates.
