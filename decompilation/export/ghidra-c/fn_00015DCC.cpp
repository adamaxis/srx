// Ghidra headless export
// Entry: 0x00015DCC
// Name: transform_entity_heading_vector
// Decompile status: Ghidra emitted an incorrect no-op; see the disassembly
// annotation below and ghidra_conversion/entity_coordinate_conversion.cpp.
// Calls recovered from direct call instructions:
//   (none)


/*
 * 0x15DCC is not a no-op. The authoritative bytes copy D2 to D4, copy the
 * magnitude word D3 to D2, select the 0x15E16/0x15E56 table pair from D4's
 * high byte, double both signed MULS products, and return their high words.
 */
void transform_entity_heading_vector(void)

{
  short unaff_D3w;
  
  if (unaff_D3w != 0) {
    return;
  }
  return;
}

