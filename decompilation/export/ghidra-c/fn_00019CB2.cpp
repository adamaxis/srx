// Ghidra headless export
// Entry: 0x00019CB2
// Name: FUN_00019cb2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x00017DB8 advance_hud_footer_offset
//   0x00019B34 publish_menu_selection_then_text
//   0x00019BAE select_hud_mode


void FUN_00019cb2(void)

{
  char cVar1;
  short extraout_D1w;
  short sVar2;
  bool bVar3;
  
  if (DAT_00fff0d3 < '\x01') {
    publish_menu_selection_then_text();
  }
  else {
    cVar1 = select_hud_mode();
    if ((cVar1 == '\b') && (DAT_00ffdf84 != '\0')) {
      bVar3 = SCARRY1(DAT_00fff0d3,DAT_00fffbca);
      DAT_00fffbca = DAT_00fff0d3 + DAT_00fffbca;
      if (DAT_00fffbca == '\0' || bVar3 != DAT_00fffbca < '\0') {
        DAT_00fffbca = '\0';
      }
      else if ('\n' < DAT_00fffbca) {
        DAT_00fffbca = '\n';
      }
    }
    else {
      advance_hud_footer_offset();
    }
    set_hud_selection_value();
    render_menu_text_followup();
  }
  sVar2 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar2] == '\x04') {
      clear_menu_slot_visual();
      (&DAT_00fff0e8)[sVar2] = 0;
      *(short *)((int)&DAT_00fff0f2 + (int)(short)(sVar2 << 1)) = extraout_D1w * 10;
      refresh_menu_number_or_grid();
      clear_menu_status_at();
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


