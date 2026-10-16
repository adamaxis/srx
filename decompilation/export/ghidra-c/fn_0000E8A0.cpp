// Ghidra headless export
// Entry: 0x0000E8A0
// Name: sample_terrain_collision_tiles
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   (none)


// Source behavior: clear the blocked-tile latch, then either sample one
// center tile or the four corners in order (-12,-7), (+12,-7), (-12,+7),
// (+12,+7). Each 0x7F is remembered as an override, and each greater-than-or-
// equal comparison replaces the current maximum, so later equal values win
// and the final blocked latch returns 0x7F. The native conversion publishes
// the four values and selected tile index as TerrainTileSample.


