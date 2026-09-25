// Ghidra headless export
// Entry: 0x000040E2
// Name: convert_field_routine_0x000040e2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003DDE emit_field_tile_row_words
//   0x00004000 convert_field_routine_0x00004000
//   0x000041F0 convert_field_routine_0x000041f0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short convert_field_routine_0x000040e2(void)

{
  short in_D0w;
  short sVar1;
  
  if (in_D0w < 0) {
    sVar1 = (DAT_00ffddc6 >> 4) + 8;
    if (sVar1 <= DAT_00ffddce) {
      convert_field_routine_0x000041f0();
      sVar1 = DAT_00ffdd08 + -0x148;
      convert_field_routine_0x00004000();
      emit_field_tile_row_words();
      _DAT_00ffdf58 = 0x8000;
      sVar1 = emit_field_tile_row_words(sVar1);
      _DAT_00ffdf58 = 0;
    }
  }
  else {
    sVar1 = (DAT_00ffddc6 >> 4) - 8;
    if (-1 < sVar1) {
      convert_field_routine_0x000041f0();
      sVar1 = DAT_00ffdd08 + 8;
      convert_field_routine_0x00004000();
      emit_field_tile_row_words();
      _DAT_00ffdf58 = 0x8000;
      sVar1 = emit_field_tile_row_words(sVar1);
      _DAT_00ffdf58 = 0;
    }
  }
  return sVar1;
}


