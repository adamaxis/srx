// Ghidra headless export
// Entry: 0x0000C8B0
// Name: FUN_0000c8b0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000c8b0(void)

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


