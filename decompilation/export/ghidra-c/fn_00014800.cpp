// Ghidra headless export
// Entry: 0x00014800
// Name: probe_obstruction_strip
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void probe_obstruction_strip(void)

{
  ushort unaff_D5w;
  short unaff_D6w;
  short unaff_D7w;
  int unaff_A4;
  
  do {
    unaff_D5w = *(short *)(&DAT_000148ca + unaff_D6w) + unaff_D5w;
    if ((((short)unaff_D5w < 0) || (DAT_00ffddea < (unaff_D5w & 0x3f))) ||
       (DAT_00ffddd4 >> 5 < unaff_D5w >> 6)) {
      return;
    }
    if (*(char *)(unaff_A4 + (short)unaff_D5w) != '\0') {
      return;
    }
  } while (*(char *)(unaff_A4 + (short)(*(short *)(&DAT_000148ca + unaff_D7w) + unaff_D5w)) != '\0')
  ;
  return;
}


