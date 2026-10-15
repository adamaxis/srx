# Pocket Secretary Lower-Bezel Animation Audit

Date: 2026-10-15
ROM: US Genesis Shadowrun
Effect: 0xE660, used by Pocket Secretary menu loops

## ROM behavior

The source menu loops wait for VBlank, sample input, then call 0xE660 once.
For example, the Pocket Secretary root calls it at 0xC2E6, Contacts at
0xF8B2, and the shared detail wait 0xF312 calls it at 0xF324. Since the
call is before the input branch, the effect continues while the player is
idle and also advances on a frame that accepts input. It is not input-triggered
and it has no fixed repeating phase: each active VBlank chooses a new cell and
pattern. 0xE660 calls the bounded random service and the VDP grid writer; it
does not request a sound effect.

One 0xE660 call consumes three source-random draws:

| Draw | Result | ROM source |
| --- | --- | --- |
| 1, bound 2 | 0 | Select one of eight 0xC13E8 destinations and a fixed pattern |
| 1, bound 2 | nonzero | Select one of four 0xC13F8 destinations and an indexed pattern |
| 2, bound 8 or 4 | table entry | Choose the destination |
| 3, bound 3 or 4 | table entry | Choose the pattern |

0x1232 writes one row of two adjacent 8x8 cells. The cells remain in the
name table until another call replaces them or the menu panel is rebuilt.
There is no phase counter or timed light-off step in this routine.

## Destination placement

The ROM words are VDP addresses. With the field plane's 0xC000 base and
128-byte tile-row stride, the two eight-entry tables resolve as follows:

| Table | Word | Screen origin |
| --- | --- | --- |
| 0xC13E8 | 0xCC8A | (40, 200) |
| 0xC13E8 | 0xCC92 | (72, 200) |
| 0xC13E8 | 0xCCAE | (184, 200) |
| 0xC13E8 | 0xCCB6 | (216, 200) |
| 0xC13E8 | 0xCD0A | (40, 208) |
| 0xC13E8 | 0xCD12 | (72, 208) |
| 0xC13E8 | 0xCD2E | (184, 208) |
| 0xC13E8 | 0xCD36 | (216, 208) |
| 0xC13F8 | 0xCC8E | (56, 200) |
| 0xC13F8 | 0xCCB2 | (200, 200) |
| 0xC13F8 | 0xCD0E | (56, 208) |
| 0xC13F8 | 0xCD32 | (200, 208) |

The presenter uses these twelve destinations only; the captured root, child,
detail, and return screens show no write across text, cursor, or the central
button cluster.

## Selector-to-art correction

The ROM fixed-family branch does not select the extracted resources in
address order:

| ROM selector | ROM table | Extracted asset | Appearance |
| ---: | --- | --- | --- |
| 0 | 0xCE136 | random-grid-fixed-2.bmp | yellow |
| 1 | 0xCE12E | random-grid-fixed-1.bmp | red |
| 2 | 0xCE126 | random-grid-fixed-0.bmp | blue |

The indexed branch selects 0xCE13E + 4*n, so selector 0 through 3 maps in
order to random-grid-indexed-0.bmp through random-grid-indexed-3.bmp.

The presenter used the source selector as a direct index into the
address-ordered fixed assets, swapping fixed selectors 0 and 2. It now loads
fixed assets in selector order while leaving the offline catalog names in ROM
address order. Lua continues to store the source selector; Qt presents that
state with the matching named art.

## Verification

Debug and Release builds pass. The gameplay script runtime, Pocket Secretary
route QA, and field menu text catalog CTests pass in both configurations.
The route QA pins the seed to 0x0012, which produces bounded results
[0, 0, 0]; it verifies that selector 0 paints the full 0xCE136 yellow image
at (40, 200) and that the source seed advances to 0x4BC9. It also captures
the 30-VBlank detail wait while checking that text and selection stay fixed.
All 95 Debug and Release full-screen BMPs have identical paths and SHA-256
hashes. Every Debug capture was reviewed in 11 full-screen sheets, including
the complete moving lower panel.

This verifies the host selector-to-art mapping and the traced call cadence. It
does not prove independent Genesis appearance: the supplied longplay has no
matching Pocket Secretary visit, and the live-game initial RNG seed remains
unrecovered. MENU-BLINK-01 therefore remains partial.
