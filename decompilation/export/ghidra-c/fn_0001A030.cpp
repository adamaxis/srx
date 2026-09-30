// Ghidra headless export
// Entry: 0x0001A030
// Name: resolve_title_world_action_selection
// Decompile status: ok
// Original Ghidra label: select_menu_entry
// Native conversion: ghidra_conversion::resolve_title_world_action_selection.
// ROM tracing restores the branch span through 0x1A09A.  The generated
// function previously stopped before the 0x55FD8 call and represented the
// 0x56050 second-roll continuation as an anonymous call; both boundaries are
// now named in the canonical export plugin and typed conversion.
// Calls recovered from direct call instructions:
//   0x00019BAE select_hud_mode
//   0x00055FD8 calculate_and_publish_runner_action_result
//   0x00056050 resolve_title_world_action_marker


undefined2 resolve_title_world_action_selection(void)

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
      uVar1 = resolve_title_world_action_marker();
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


