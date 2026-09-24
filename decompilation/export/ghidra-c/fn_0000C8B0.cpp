// Ghidra headless export
// Entry: 0x0000C8B0
// Name: apply_byte_counter_delta_with_negative_floor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void apply_byte_counter_delta_with_negative_floor(void)

{
  char cVar1;
  char unaff_D6b;
  short unaff_D7w;
  
  cVar1 = unaff_D6b + (&DAT_00fffc12)[(short)(unaff_D7w + -1)];
  (&DAT_00fffc12)[(short)(unaff_D7w + -1)] = cVar1;
  if (cVar1 < '\0') {
    (&DAT_00fffc12)[(short)(unaff_D7w + -1)] = 0xff;
  }
  return;
}


