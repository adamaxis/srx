// Ghidra headless export
// Entry: 0x0001792E
// Name: render_menu_hud_rows
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00017C3C convert_world_routine_0x00017c3c
//   0x00056506 convert_intro_routine_0x00056506


void render_menu_hud_rows(void)

{
  short sVar1;
  short sVar2;
  int iVar3;
  
  sVar2 = 4;
  iVar3 = 0xebc0;
  do {
    sVar1 = sVar2;
    if (((DAT_00fff0b0 != '\0') && (convert_world_routine_0x00017c3c(sVar2,iVar3), sVar2 != 0)) &&
       (-1 < (char)(&DAT_00fffbd8)[sVar1])) {
      convert_intro_routine_0x00056506();
    }
    write_vdp_grid();
    iVar3 = iVar3 + -0xc;
    sVar2 = sVar1 + -1;
  } while (sVar2 != -1);
  return;
}


