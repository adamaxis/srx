// Ghidra headless export
// Entry: 0x00011624
// Name: render_equipment_highlights
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00011136 render_menu_tile_rows
//   0x00011678 write_menu_grid
//   0x000116C4 write_menu_grid_quad


undefined2 render_equipment_highlights(void)

{
  undefined2 uVar1;
  short sVar2;
  int extraout_A0;
  int iVar3;
  
  uVar1 = write_menu_grid_quad();
  iVar3 = DAT_00fffc04;
  if ((-1 < *(char *)(DAT_00fffc04 + 0x56)) && (*(char *)(DAT_00fffc04 + 0x56) != '\x7f')) {
    write_menu_grid();
    iVar3 = extraout_A0;
  }
  if (*(char *)(iVar3 + 0x76) != '\0') {
    sVar2 = 7;
    do {
      if ((char)(*(char *)(iVar3 + 0x76) + '+') == *(char *)(iVar3 + 0x66 + (int)sVar2)) {
        write_menu_grid();
        break;
      }
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  }
  render_menu_tile_rows();
  return uVar1;
}


