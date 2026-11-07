# Scene 2 Shiawase and Halloweeners wall layers

Status: **partial**. The separator pattern by the Shiawase marker candidate
matches the independent longplay capture and the ROM-backed Scene 2 tilemap.
No runtime or asset fix is supported by this comparison. A GPGX view of the
Halloweeners target area has not yet been identified, and the original report
does not give exact tile coordinates for every disputed separator.

## ROM map path

- [C] Scene 2 is selected through the mode table at `0x323A`, whose ROM long
  pointer is `0x172C80`. That profile's `+0x14/+0x18/+0x1C` pointers select
  primary map `0x16D9A2`, metatile stream `0x170102`, and secondary map
  `0x16ED52`; setup block `0x172C50` supplies pattern data `0x166B82`. The
  decoded map is 60 by 42 cells. These values agree with row 2 in
  `decompilation/assets/field/maps/index.tsv`.
- [C] Scene-2 terrain record 95 at `0x173160` is
  `0440 045F 04A0 04BF 00C3`. Its 32-pixel map cell is column 34, row 37
  (map pixels x `1088..1119`, y `1184..1215`). The project labels nearby guide
  marker 10 “Shiawase” only as a candidate; the name is not stored in this
  record.
- [C] At map row 37, column 34, the primary-map word at `0x16EB3E` is
  `0x0138`; the secondary-map word at `0x16FEEE` is `0x020E`. In row 38,
  columns 31, 33, 34, and 36 use primary `0x00A1` with secondary `0x0223` at
  primary addresses `0x16EBB0/0x16EBB4/0x16EBB6/0x16EBBA` and secondary
  addresses `0x16FF60/0x16FF64/0x16FF66/0x16FF6A`. Columns 32 and 35 instead
  use secondary `0x020E` at `0x16FF62/0x16FF68`. The decoded secondary layer
  has 150 visible pixels for each `0x0223` cell and none for `0x020E`, so the
  repeated wall details have deliberate gaps.
- [C] ROM `0x3D80` uses `FFDF58` while writing name words: its negative path
  XORs `0x8000`, while the other path ORs the flag. The offline map extractor
  applies that rule when it creates `mode-2.bmp` and
  `mode-2-priority-overlay.png`; the Qt field renderer draws sparse priority
  pixels after ordinary low-priority actors.

## Independent frame comparison

The independent longplay frame
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_10_source_1320-1620/frame_001800.png`
is source time 23:00. Aligning its field viewport to Scene 2 places the map
origin at `(960,1088)`. Row 38 therefore appears at screen y `128`; columns
31-36 appear at x `32, 64, 96, 128, 160, 192`. The captured wall shows the
same repeated pipe/separator pieces and empty intervals as the `0x0223` and
`0x020E` secondary-map entries. Unoccupied 32-by-32 cells compare closely to
the extracted map; the cell at column 34 is actor-covered and is excluded
from that static comparison.

Frame
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/section_10_source_1320-1620/frame_000000.png`
at 22:00 gives a second view from map origin `(842,1088)`. Together, these
frames support the Shiawase-side wall placement and priority artwork. They do
not show the Halloweeners target record 94 at `0x173156` (map cells
`x=1664..1695`, `y=1152..1183`), so that side remains unverified against
longplay.

## Remaining

The extracted map and the two GPGX views agree on the inspected Shiawase-side
separators. This audit does not establish whether another reported "odd slot"
refers to a different cell or to the Halloweeners target. Keep playtest item
#51 partial until that location is compared against an independent frame or
the exact reproduction coordinates are recorded.
