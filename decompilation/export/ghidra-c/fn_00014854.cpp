// Ghidra headless export
// Entry: 0x00014854
// Name: can_bypass_obstruction
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void can_bypass_obstruction(void)

{
  short sVar1;
  short unaff_D5w;
  ushort unaff_D6w;
  short unaff_D7w;
  int unaff_A4;
  
  sVar1 = unaff_D5w;
  while (sVar1 = *(short *)(&DAT_000148ca + (short)unaff_D6w) + sVar1,
        *(char *)(unaff_A4 + sVar1) == '\0') {
    if (*(char *)(unaff_A4 + (short)(*(short *)(&DAT_000148ca + unaff_D7w) + sVar1)) == '\0') {
      return;
    }
  }
  do {
    unaff_D5w = *(short *)(&DAT_000148ca + (short)(unaff_D6w ^ 4)) + unaff_D5w;
    if (*(char *)(unaff_A4 + unaff_D5w) != '\0') {
      return;
    }
  } while (*(char *)(unaff_A4 + (short)(*(short *)(&DAT_000148ca + unaff_D7w) + unaff_D5w)) != '\0')
  ;
  return;
}


