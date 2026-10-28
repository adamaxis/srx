# Player hit spark (routine 0xAC4E): ROM-rendered frames vs port BMPs

Scope: player-side spark spawned by 0xB280/0xB2B2 (child from 0x15A8C). The
combat branch also identified 0xAC4E as the impact sprite; this audit covers
only the visual frames/timing, not who spawns it (see HANDOFF overlap note).

## Method (emulator, ROM renders the frames)
No saved state has an enemy in melee range (a 23-state × A/B/C × 900-frame scan
for `00 00 AC 4E` in RAM found no spark), so the spark is injected exactly as
0xB2B2..0xB2EA writes it, and the ROM's own object/SAT code animates and draws it:

- free slot from 0x15A8C's pool: `FF0B00 + n*0x100` (n=0..5, child index 9+n),
  free when `+0x10` bit 0 is clear [C]
- zero the record, then `+0x3E`=index, `+0x16`=4, `+0xE`=1, `+0x3A`=0x0000AC4E,
  `+0x10`=1, `+0x11` bit0, X = leader X, Y = leader Y + 0x10, `+0x2C`=0x2000 [C]
- leader record = long at FFFC04 (FF0100 in all tested states)
- run 39 frames, diff each frame against an un-injected run from the same state

Script: `tools/rom_research/emu_probes/spark_inject.py <state> <outdir>`;
sheet: `spark_sheet.py`. Output (not committed): `shadowrun/out/hit_spark_compare/`
(`contact_sheet.png`, per-state `inj_NN.png` / `base_NN.png`).

## Findings
- [C] Lifetime: the slot stays live for 5 object ticks; `+0x16` gains the
  finished bit (0x8004) on tick 5 (`E.HitSpark.FINISHED_SELECTOR_BIT`), and the
  slot frees on the next tick. Visible pixels appear on 4-5 consecutive displayed
  frames (s5: frames 1-5; hw1: frames 2-5). This matches FRAME_COUNT=5 with
  FRAME_INCREMENT 0x400 / PERIOD 0x1000 (one list step per tick).
- [C] Palette: every changed pixel uses the same 3-bit CRAM levels as the port
  BMPs. GPGX expands levels as 0/32/98/172/238; the port uses 0/36/109/182/255
  (e.g. 238,0,0 ↔ 255,0,0; 172,32,32 ↔ 182,36,36; 238,238,238 ↔ white). There
  is no palette mismatch.
- [C] Shape progression: the ROM frames show the same growing red/white burst as
  hit-spark-frame-0..4.bmp (7×5 → 11×12 → 15×16 → 16×16 → 16×16 ring); see the
  contact sheet.
- [C] Priority: `+0x2C`=0x2000 leaves the sprite priority bit (bit 15) clear,
  so the ROM spark is a low-priority sprite. In both test scenes it is mostly
  hidden behind high-priority plane tiles (hw1 shows only slivers, 6-17 px; s5
  shows 14-40 px). The port's spark is pushed into the generic SceneSprite list
  (field_scene_runtime.cpp, the `field_hit_sparks_` loop before the 0x5B1E sort),
  so whether it is occluded depends on that path's high-priority overlay handling.
  [H] Check this against the ROM in the combat-owned paint_field_scene; core did
  not change it.
- Not checked: the exact pixel offset of each frame's origin against the ROM. That
  needs a non-occluded spot (open floor with no high-priority tiles); the s5 crops
  roughly agree but are partly covered.

## Independent longplay search

A full-rate template search checked 19,860 numbered 320x224 frames from sections
04, 06, 07, and 25 of
`video analysis/long_gameplay_video_early_sections_ffmpeg_30fps_320x224/` and
`video analysis/long_gameplay_video_rest_ffmpeg_30fps_320x224/`. The ROM-derived
BMP masks were used only to find candidate frames; the longplay frames remain
the independent visual source.

One plausible burst appears in section 25, frames 001133-001143. In the active
field-combat scene, a brief red/white flash sits between the two actors. At the
same location, the best mask matches progress through spark stages 1, 2, and 3:
frames 001135-001136, 001137-001138, and 001139-001140 respectively. There is
also a possible stage-0 match at frame 001133. The stage-1 through stage-3
matches remain near `(137,95)` in the frame, with small position differences as
the shape grows.

This is a candidate visual correspondence, not a pixel-exact confirmation. Red
target brackets overlap the effect area and contribute to the mask score; the
video is sampled at 30 fps, and there is no controller or event log to associate
the flash with a specific ROM hit or actor. The capture does not settle the
origin offset, source-tick phase, hidden frames, or the stage-4 ring. Keep those
questions open; the function remains partial.
