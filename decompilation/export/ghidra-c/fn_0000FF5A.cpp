// Ghidra headless export
// Entry: 0x0000FF5A
// Name: convert_graphics_routine_0x0000ff5a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 upload_field_asset_setup
//   0x00003548 draw_hud_panel_triplet
//   0x000036FC advance_entity_asset_meter
//   0x000037EC advance_entity_behavior_meter
//   0x00010016 convert_menu_routine_0x00010016
//   0x000102D4 convert_menu_routine_0x000102d4
//   0x00010722 convert_menu_routine_0x00010722
//   0x000125FA clear_entity_auxiliary_marker


void convert_graphics_routine_0x0000ff5a(void)

{
  undefined1 extraout_D1b;
  undefined1 unaff_D7b;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  convert_menu_routine_0x000102d4();
  iVar1 = DAT_00fffc04;
  if ((-1 < *(char *)(DAT_00fffc04 + 0x56)) && (*(char *)(DAT_00fffc04 + 0x57) != '\0')) {
    clear_entity_auxiliary_marker();
    *(undefined1 *)(extraout_A0 + 0x4e) = extraout_D1b;
    iVar1 = extraout_A0;
  }
  *(undefined1 *)(iVar1 + 0x57) = unaff_D7b;
  *(char *)(iVar1 + 0x57) = *(char *)(iVar1 + 0x57) + '\x01';
  while (((convert_menu_routine_0x00010016(), (DAT_00ffdd22._0_1_ & 0x20) == 0 &&
          ((DAT_00ffdd22._0_1_ & 0x10) == 0)) && ((DAT_00ffdd22._0_1_ & 0x40) == 0))) {
    convert_menu_routine_0x00010722();
    if ((((DAT_00ffdd22._0_1_ & 0x20) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
       ((DAT_00ffdd22._0_1_ & 0x40) != 0)) break;
  }
  *(undefined1 *)(DAT_00fffc04 + 0x56) = 0xff;
  upload_field_asset_setup();
  draw_hud_panel_triplet();
  advance_entity_behavior_meter();
  advance_entity_asset_meter();
  *(char *)(extraout_A0_00 + 0x9d) = (char)DAT_00fff096;
  return;
}


