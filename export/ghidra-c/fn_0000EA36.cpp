// Ghidra headless export
// Entry: 0x0000EA36
// Name: evaluate_entity_terrain_collision_directions
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x0000EAD2 evaluate_terrain_direction_boundary


// Source behavior: inspect FFEF10, FFEF11, FFEF12, and FFEF13 in order;
// only entries with bit 5 set call 0xEAD2. Their probe offsets are exactly
// (-12,-7), (+12,-7), (-12,+7), and (+12,+7), and the first carry returns to
// the caller. The native conversion is
// evaluate_entity_terrain_collision_directions(), which uses the typed sample
// and selected terrain window.


