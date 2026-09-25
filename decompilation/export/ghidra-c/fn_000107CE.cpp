// Ghidra headless export
// Entry: 0x000107CE
// Name: convert_menu_routine_0x000107ce
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0001055E convert_menu_routine_0x0001055e
//   0x000105E6 convert_menu_routine_0x000105e6
//   0x00010854 write_three_grids_then_service


void convert_menu_routine_0x000107ce(void)

{
  char cVar1;
  int iVar2;
  char in_D0b;
  
  iVar2 = DAT_00fffc04;
  cVar1 = in_D0b + *(char *)(DAT_00fffc04 + 0x4d);
  *(char *)(DAT_00fffc04 + 0x4d) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(iVar2 + 0x4d) = 0;
  }
  else if ('\x04' < *(char *)(iVar2 + 0x4d)) {
    *(undefined1 *)(iVar2 + 0x4d) = 4;
  }
  write_vdp_grid();
  render_encoded_title_text();
  write_three_grids_then_service();
  convert_menu_routine_0x000105e6();
  convert_menu_routine_0x0001055e();
  convert_menu_routine_0x0001055e();
  return;
}


