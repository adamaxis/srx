// Ghidra headless export
// Entry: 0x0001E41E
// Name: FUN_0001e41e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 FUN_0001e41e(void)

{
  ushort unaff_D5w;
  ushort unaff_D6w;
  
  return (&DAT_00ff1700)[(short)((unaff_D5w >> 9) + (unaff_D6w >> 4 & 0x7fe0) * 2)];
}


