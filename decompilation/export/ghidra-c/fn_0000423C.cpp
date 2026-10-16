// Ghidra headless export
// Entry: 0x0000423C
// Name: quantize_field_delta
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short quantize_field_delta(void)

{
  short in_D0w;
  
  return ((short)(in_D0w - DAT_00ffddc6) >> 7) << 1;
}


