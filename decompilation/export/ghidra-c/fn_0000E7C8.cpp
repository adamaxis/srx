// Ghidra headless export
// Entry: 0x0000E7C8
// Name: evaluate_terrain_collision
// Decompile status: reviewed against export/disassembly.s and the US ROM.
// Calls recovered from direct call instructions:
//   0x0000E8A0 sample_terrain_collision_tiles
//   0x0000E9EE contains_terrain_object_bounds
//   0x0000EA36 evaluate_entity_terrain_collision_directions


// Source behavior:
//   clear the outside/directional markers, shift D5/D6 right four bits for
//   map-bound checks and for all callees, then call 0xE8A0 with A0+0x10 bit 2
//   as the center-versus-four-corner probe selector. A zero or 0x7F sample is
//   returned directly; signed negative samples also pass through. Positive
//   object codes use 0xE9EE, while directional codes use 0xEA36. Bit 6 arms
//   the directional marker, 0x7E and bit-5-clear values return blocked, and
//   a successful boundary callback maps to return code 0x7F.
// The native conversion publishes this as TerrainCollisionResult with typed
// callbacks for decoded tile/object data; it does not expose source RAM.


