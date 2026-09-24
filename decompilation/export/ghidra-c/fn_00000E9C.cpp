// Ghidra headless export
// Entry: 0x00000E9C
// Name: FUN_00000e9c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00000e9c(void)

{
  ushort in_D0w;
  
  VDP_CTRL = in_D0w & 0x3fff | 0x4000;
  VDP_CTRL = in_D0w >> 0xe;
  return;
}


