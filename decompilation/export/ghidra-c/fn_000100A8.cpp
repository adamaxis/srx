// Ghidra headless export
// Entry: 0x000100A8
// Name: adjust_magic_force
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00010156 write_repeated_menu_grid
//   0x00010172 refresh_magic_force_display
//   0x0001055E refresh_equipment_combat_values
//   0x000105E6 refresh_equipment_action_metric
//
// Source correction: the disassembly pushes only FFF096 before
// refresh_equipment_combat_values and restores only that word afterward.
// The generated locals below incorrectly preserve FFF098 as well; the typed
// conversion intentionally leaves the refreshed maximum-force value intact.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void adjust_magic_force(void)

{
  short sVar1;
  short sVar2;
  short in_D1w;
  
  DAT_00fff096 = in_D1w + DAT_00fff096;
  if (DAT_00fff098 < DAT_00fff096) {
    DAT_00fff096 = DAT_00fff098;
  }
  if (DAT_00fff096 == 0) {
    DAT_00fff096 = 1;
  }
  *(char *)(DAT_00fffc04 + 0x9d) = (char)DAT_00fff096;
  build_grouped_decimal_glyphs();
  refresh_magic_force_display();
  write_repeated_vdp_grid();
  sVar2 = DAT_00fff098;
  sVar1 = DAT_00fff096;
  refresh_equipment_combat_values();
  DAT_00fff098 = sVar2;
  DAT_00fff096 = sVar1;
  write_repeated_menu_grid();
  refresh_equipment_action_metric();
  return;
}


