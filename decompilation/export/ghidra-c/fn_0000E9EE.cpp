// Ghidra headless export
// Entry: 0x0000E9EE
// Name: contains_terrain_object_bounds
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   (none)


// Source behavior: select the object record by ten-byte stride, then test
// signed X within center +/-12 and signed Y within center +/-7, inclusive.
// The native conversion exposes this through TerrainObjectLookup and does not
// reproduce the source record pointer arithmetic.


