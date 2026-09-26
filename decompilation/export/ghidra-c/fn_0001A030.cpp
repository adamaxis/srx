// Ghidra headless export
// Entry: 0x0001A030
// Name: select_menu_entry
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019BAE select_hud_mode
//   0x00055FD8 calculate_and_publish_runner_action_result


undefined2 select_menu_entry(void)

{
  char cVar2;
  undefined2 uVar1;
  undefined1 *unaff_A4;
  undefined1 uVar3;
  bool bVar4;
  
  uVar3 = DAT_00ffdf84 < '\0';
  bVar4 = DAT_00ffdf84 == '\0';
  if ((bVar4) &&
     ((cVar2 = select_hud_mode(), bVar4 ||
      ((cVar2 != '\x01' && (uVar3 = (char)(cVar2 + -8) < '\0', cVar2 != '\b')))))) {
    calculate_and_publish_runner_action_result();
    if (!(bool)uVar3) {
      uVar1 = func_0x00056050();
      DAT_00fff0aa = *unaff_A4;
      return uVar1;
    }
    prepare_menu_offset_once();
    return 0xffff;
  }
  set_hud_selection_value();
  render_menu_text_followup();
  prepare_menu_offset_once();
  return 0x45;
}


