// Ghidra headless export
// Entry: 0x0000EAD2
// Name: evaluate_terrain_direction_boundary
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x0000EDFA decode_terrain_tile_window


// Source behavior: 0xEDFA expands the selected tile index into origin X,
// origin Y, and end Y. After masking the code with 0x38, the four branches
// compare wrapped unsigned word deltas using BCS/BCC. The typed conversion
// keeps those exact unsigned comparisons in
// evaluate_terrain_direction_boundary().


