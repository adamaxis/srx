// Ghidra headless export
// Entry: 0x00011950
// Name: run_cyberware_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x00011900 prepare_menu_row_grid
//   0x00011928 render_menu_row
//   0x00012C36 prepare_menu_grid_variant_b


void run_cyberware_menu(void)

{
  short sVar1;
  undefined4 unaff_D3;
  short sVar2;
  
  prepare_menu_grid_variant_b();
  prepare_menu_row_grid();
  render_encoded_title_text();
  sVar1 = *(short *)(DAT_00fffc04 + 0x90);
  if (sVar1 == 0) {
    render_encoded_title_text();
  }
  else {
    sVar2 = 0xe;
    do {
      if ((CONCAT22((short)((uint)unaff_D3 >> 0x10),sVar1) & 1 << ((ushort)(0xe - sVar2) & 0x1f)) !=
          0) {
        render_encoded_title_text();
      }
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  }
  wait_for_controller_event();
  render_menu_row();
  return;
}


