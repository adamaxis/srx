// Ghidra headless export
// Entry: 0x0000E9EE
// Name: contains_terrain_object_bounds
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   (none)


// Source behavior: select the same five-word field terrain record used by
// 0xEE10 through its ten-byte object-id stride. The first long supplies the
// inclusive left/right X boundaries and the second long supplies the
// inclusive top/bottom Y boundaries; the probe expands them by +/-12 and
// +/-7 before the signed word comparisons. The native conversion resolves
// that record into TerrainObjectBounds rather than exposing the RAM pointer.


