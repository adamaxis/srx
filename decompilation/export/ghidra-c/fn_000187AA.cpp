// Ghidra headless export
// Entry: 0x000187AA
// Name: render_hud_indicator_grids
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void render_hud_indicator_grids(void)

{
  short sVar1;
  short sVar2;
  
  sVar2 = 0;
  sVar1 = 0xb;
  do {
    if ((&DAT_00fffbcc)[sVar2] != '\0') {
      write_vdp_grid();
    }
    sVar2 = sVar2 + 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar2 = 3;
  do {
    write_vdp_grid();
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


