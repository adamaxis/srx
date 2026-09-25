// Ghidra headless export
// Entry: 0x00011592
// Name: convert_menu_routine_0x00011592
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0000C0DA render_encoded_title_text
//   0x00011ABE prepare_menu_dialog
//   0x00012148 read_entity_action_code
//   0x00012A8C render_item_action_details
//   0x00012C36 prepare_menu_grid_variant_b
//   0x00012E9E service_entity_action_menu


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_menu_routine_0x00011592(void)

{
  char cVar1;
  undefined1 uVar2;
  
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  _DAT_00ffd980 = 1;
  uVar2 = (DAT_00ffef14 & 2) == 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  prepare_menu_grid_variant_b();
  read_entity_action_code();
  if (!(bool)uVar2) {
    cVar1 = render_item_action_details();
    if ((char)(cVar1 + -0x10) < '\0') {
      service_entity_action_menu();
    }
    prepare_menu_dialog();
    return;
  }
  render_encoded_title_text();
  return;
}


