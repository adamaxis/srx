// Ghidra headless export
// Entry: 0x000148AA
// Name: find_first_occupied_obstruction_tile
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short find_first_occupied_obstruction_tile(void)

{
  short unaff_D5w;
  short unaff_D7w;
  int unaff_A4;
  
  do {
    unaff_D5w = *(short *)(&DAT_000148ca + unaff_D7w) + unaff_D5w;
  } while (*(char *)(unaff_A4 + unaff_D5w) == '\0');
  return *(short *)(&DAT_000148ca + unaff_D7w);
}


