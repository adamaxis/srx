# Field barrier (Barrier / Super Barrier): presentation and collision notes

Status: notes only. The port already handles the cast, the slot reservation
(`kFieldPlayerBarrierEffectOwner`) and the Lua lifetime clock (actions.lua,
`E.SpellEffectKind.FIELD_BARRIER`, `FIELD_BARRIER_TICKS_PER_SUCCESS`). Missing:
the five floor cells written into FF1700, the five-segment sprite, and the
restore on expiry or scene change. Everything below was checked with m68dis on
rom.bin [C], unless marked [H].

## Record and dispatch
- [C] 0x15830: spell codes 7 (Super Barrier) and 14 (Barrier) jump to 0x49A2.
- [C] There is one global record at FFDC9E. `+0x02` != 0 means active. 0x49A2
  first tears down an existing barrier (`bsr 0x4B6C`) and then allocates a new one.
- [C] Allocation: 0x1582A → 0x1A2CC scans 18 bytes at FFD86C for a free slot,
  marks it 1, and returns d0 = slot (stored at `+0x32`) and d1 = 0x49 + 0x18*slot
  (the VRAM tile base, stored at `+0x2A`). If the pool is full it sets carry and
  returns with no barrier. The port's pre-clear of the owner slot matches this.
- [C] `+0x0E` lifetime = FFF0DD byte × 0x190 (0x49EC..0x49F6).
- [C] `+0x06` = caster heading octant `((+0x2E rol 3) & 7)`.
- [C] Centre position: caster X/Y plus the table at 0x4AAC[octant], which is
  (dx,dy) = N(0,-1152) NE(814,-814) E(1152,0) SE(814,814) S(0,1152)
  SW(-814,814) W(-1152,0) NW(-814,-814). Each axis is then snapped to the
  cell centre: `(v & 0xFE00) + 0x100` (E.Grid.CELL_CENTER). The result goes to
  `+0x00` / `+0x04`.
- [C] Base cell index = `(y >> 9) * 64 + (x >> 9)` (E.Grid.ROW_CELLS /
  CELL_SHIFT), stored at `+0x34`.

## Collision cells (0x4A44..0x4A86)
- [C] Five cells are base + the word list at 0x4BB0[octant] (cell units; ±1 = x,
  ±64 = row). Lists are 0x4BD0..0x4C02:
  - N/S (0,4): `0, +1, -1, +2, -2` (horizontal wall)
  - E/W (2,6): `0, -64, +64, -128, +128` (vertical wall)
  - NE (1): `0, -1, -2, +64, +128`
  - SE (3): `0, -1, -2, -64, -128`
  - SW (5): `0, +1, +2, -64, -128`
  - NW (7): `0, +1, +2, +64, +128`
  So the diagonal octants make an L with its corner at the centre cell.
- [C] For each cell the original FF1700 byte is saved at `+0x36 + i` and the cell
  is overwritten with 0x7E. If the caster's `+0x57` <= 9 (Super Barrier, code 7)
  the cell gets 0x7F instead. Barrier (code 14) leaves 0x7E.
- [C] The movement probe 0xE8A0 takes the highest of the four corner bytes and
  latches FFDD28 = 0x7F whenever any corner is 0x7F, which overrides the result
  (0xE9BA). The port's `terrain_probe_conversion.cpp` already models this
  (`contains_blocked_tile ? 0x7F : highest`), and the QA contact route uses raw
  0x7E. So the only missing piece is a mutable overlay on
  `field_scene_buffers.source_metadata.terrain_collision_map`.
- [H] I have not traced how 0x7E differs from 0x7F downstream (e.g. projectile
  or line-of-sight probes vs actor movement). Assume both block walking.

## Sprite (0x4ADC per frame, called from 0x47EC while `+0x02` != 0)
- [C] `subq #1,+0x0E`; going negative branches to the 0x4B6C teardown.
- [C] 0x5A56 projects the world position to screen coordinates `+0x0A` / `+0x0C`.
- [C] The animation frame is 0x4ACC[`+0x0E` & 3], i.e. 0x5802 / 0x5814 / 0x5826 /
  0x5838, giving a 4-frame loop at one step per frame. The frame's sprite list
  (after an 8-byte header) is drawn by 0x13BE.
- [C] There are 5 segments at pixel offsets 0x4C0C[octant], which match the cell
  layout at 32 px per cell. For example N/S is (0,0) (+32,0) (-32,0) (+64,0)
  (-64,0). The attribute base is `+0x2A`, palette word d6 = 0x2000 (palette 1,
  priority bit clear, so low priority).
- [C] Flicker: with k = `+0x0E` mod 5, only segments k and (k+1) mod 5 are drawn
  each frame (0x4B2E..0x4B4C). Each frame shows 2 of 5 segments, rotating.
- [C] 0x61A0 with a2 = 0x5802 queues a DMA: source long at list+2, length word
  at list+6, destination VRAM tile `+0x2A` × 32. Barrier art is streamed per
  slot. On cast, 0x4AA0 also posts event/sfx 0x3B through 0x530DE (E.* FIELD_3B).

## Teardown 0x4B6C
- [C] Clears `+0x02`, frees the FFD86C slot (0x1A304), and writes the five saved
  bytes back into FF1700, using the same 0x4BB0 offsets.
- [C] Callers:
  - 0x49B6 (recast)
  - 0x4AE0 (timer expiry)
  - 0x2934 (frame path when FFDF8A != 0)
  - 0x16AB8 (scene load, after FFDDC4 is set from the scene table)

## Port plan (for whoever implements; player-side)
1. Lua: a pure function giving octant, centre, the five cell indices and the
   cell code (0x7E/0x7F) from caster X/Y/heading/action code. Add the tables
   0x4AAC/0x4BB0/0x4C0C as `E.FieldBarrier.*` with their addresses.
2. C++: a per-scene `std::array<std::optional<uint8_t>,5>` overlay applied in the
   `terrain_collision_map` lookup lambda (field_scene_runtime.cpp ~9128).
   Restore it on expiry, on recast, and on scene load (0x16AB8 parity).
3. Sprite: 5 SceneSprites with 2-of-5 flicker and a 4-frame loop. Extract frames
   0x5802..0x5838 first. This touches the sprite list in paint_field_scene, so
   it waits until after the combat integration.
