# Blocking story-page sequence source audit

Date: 2026-10-22

This note records the shared ROM path used by the Sinsearach Harlequin and
Spirit Eyes story-page sequences. It describes the source contract for a
future host presentation. The loop is not a runtime implementation plan for
68000 registers, VDP transfers, or ROM pointer walking.

## Caller inputs

The two currently catalogued sequences select their page table and resource
profile before calling the shared routine at `0x5D1F6`:

| Route | Setup helper | `FFF0BE` | Ordered page table | Resource profile |
| --- | --- | ---: | --- | --- |
| Sinsearach / Harlequin exposition | `0x5D130` | `9` | `0x0F5870` | `0x1FE690` |
| Spirit Eyes ritual | `0x5D172` | `7` | `0x0F4C20` | `0x1FE6C2` |

The Harlequin table contains `0x0FBAFB`, `0x0FBBBD`, and `0x0FBC6E`, then
`FFFFFFFF`. The Spirit Eyes table contains `0x0F7915`, `0x0F798D`,
`0x0F79F0`, and `0x0F7A8B`, then `FFFFFFFF`. These are ordered longword
pointers to encoded story text. The host catalog already names the text and
preserves this order in `gameplay/dialogue/cinematics.json`.

Both setup helpers call `0x5D1F6`, then call `0x53126` and write `FFE172=FF`
before returning. The handler also adjusts the mode-specific `FFF0BE` value
before entry (`0x5D130` selects 9; `0x5D172` selects 7). These source facts
map to a typed presentation request and return to the caller's field context;
the host must not add a second mutable copy of those RAM locations.

## Shared page loop

Before scanning A5, ROM `0x5D1F6` calls `0x1406`, `0x12F6`, and `0xC0C2`.
The Ghidra export cross-check labels those callees as resource preparation,
palette-fade progression, and font loading. The page loop then reads the
ordered pointer table at A5:

1. It reads the current longword and exits the loop when its sign bit is set;
   this is the `FFFFFFFF` terminator in both catalogued tables.
2. It advances A5 by one longword and passes that pointer to `0xC0DA` with
   D1=`0x6000` and D2=`0x2106`.
3. It waits in `0x0D94` for a controller event, calls the page follow-up at
   `0x5D24A`, restores A5, and checks the next page.
4. At the terminator, it calls `0x195C` and `0x131C`, then returns to its
   caller.

The exported Ghidra C for `0x0D94` cross-checks the ROM call sequence: it
services VBlank and samples the controller until `(FFDD22 & FF00) != 0`.
This is a per-page button-event wait, not a wall-clock page duration. At
`0x5D24A`, the ROM sets D4=`0x2000`, D1=`FFF0BE`, and D7=`0x27`, then calls
`0x12C5A`. That routine selects source word `0x05AB` at `0x0C1320`, ORs it
with `0x6000`, and calls `0x11C0` to write 40 repeated words per row, stepping
the destination by `0x80` each row. The row count is `FFF0BE + 1`: ten rows
for the Harlequin sequence and eight for Spirit Eyes. This repeated region
write is source behavior between pages; it is not enough to identify the
cinematic's full background or to justify a VDP-shaped host renderer.

## Resource extraction boundary

ROM `0x1406` reads each profile as a VDP setup, a palette pointer, four mode
words, a terminated pattern-upload list, then a terminated layout list. Both
story-page profiles share VDP setup `0x1FE638`; the Harlequin palette is
`0x1FE726`, and the Spirit Eyes palette is `0x1FE8A6`. The four mode words
are zero in both records.

Each pattern-upload list has one odd-tagged compressed stream. The Harlequin
profile points to `0x1F8919`; the Spirit Eyes profile points to `0x1FA8FD`.
After clearing the low-bit type tag, `0x1406` passes VDP command `0x3020` and
zero to `0x1530`, then reaches the list terminator. The following layout-list
stream is also odd-tagged: `0x1FA81F` for Harlequin and `0x1FCB19` for Spirit
Eyes. Their source parameters are respectively `{0x506, 0x24, 0x11, 0}` and
`{0x600, 0x27, 0x0F, 0}`; `0x1406` clears the type tag and calls `0x1728`.
These are distinct source stages, not two pattern-transfer records.

The existing `--field-mode-profile-sprite` importer does not accept these
records directly. It expects a different three-pointer field portrait record
(`palette`, `pattern stream`, `sprite stream`) and fixes that portrait's
pattern bank at `0x57A0`. Passing either story-page profile to it produced a
fully transparent frame because the first two words were interpreted as
palette/pattern pointers. That output is discarded and is not a cinematic
asset.

`tools/rom_research/extract_cinematic_story_art.py` now imports the two
catalogued profiles offline. It expands the tagged `0x1530` pattern stream,
checks each `0x1728` tile reference against that profile's uploaded tile
range, resolves the profile CRAM palette, and writes the named 320x224 host
images `assets/cinematics/salish-sinsearach-frosty-harlequin.bmp` and
`assets/cinematics/salish-spirit-eyes-ritual.bmp`. Their art rectangles are
37x18 tiles at pixel (24,80) and 40x16 tiles at (0,96), respectively. Each
asset is the source layout over a black host canvas; the runtime does not
decode ROM streams or reproduce VDP operations.

The page text contract is represented in `cinematics.json`. The shared loop
calls `0xC0DA` with placement word `0x2106`, which resolves to pixel (24,16)
on the 64-column plane. Page text retains its source-derived spaces and `|`
row breaks. D1=`0x6000` selects palette bank 3; CRAM entry 48 is `0x0E0E` in
both profiles, so the text ink is magenta. The clear routine writes ten rows
for FFF0BE=9 and eight rows for FFF0BE=7; after the two-row text origin this
leaves eight Harlequin rows and six Spirit Eyes rows. The host catalog records
those bounds and RGB ink instead of retaining a palette or plane simulator.

Lua now owns each ordered page operation and yields a typed story-page request
with its named art alias and text key. The Qt dialogue presenter draws that
art with the extracted field font and waits for any focused controller event
before Lua advances. After the last page, `play_cinematic` completes through
the typed source-context return handler. The routes remain partial: route
CTest and CLI `actual.bmp` evidence have not been added, and no independent
Genesis encounter frame has been identified. These BMPs are source-derived
assets, not independent visual proof.

## Source references

- US ROM: `0x5D130`, `0x5D172`, `0x5D1F6`, `0x5D24A`, `0x1406`, `0x12F6`,
  `0xC0C2`, `0xC0DA`, `0x0D94`, `0x195C`, `0x131C`, `0x12C5A`, `0x11C0`,
  `0x0E9C`, `0x0CEC`, `0x1530`, `0x1728`, `0x0C1320`, `0x0F5870`,
  `0x0F4C20`, `0x1FE638`, `0x1FE690`, and `0x1FE6C2`.
- Ghidra cross-check: `fn_00000D94.cpp`, `fn_00001406.cpp`,
  `fn_000566F6.cpp`, `fn_0000C0DA.cpp`, and `fn_000011C0.cpp`.
- Host content: `gameplay/dialogue/cinematics.json`.
