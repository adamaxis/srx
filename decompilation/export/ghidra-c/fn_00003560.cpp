// Ghidra headless export
// Entry: 0x00003560
// Name: refresh_hud_panel_variant
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003602 run_hud_panel_post_hook
//   0x00003648 draw_hud_panel


void refresh_hud_panel_variant(void)

{
  short in_D0w;
  undefined *puVar1;
  bool in_ZF;
  undefined1 *puVar2;
  
  if (in_ZF) {
    puVar1 = &DAT_000086a4;
    puVar2 = &LAB_0000f448;
  }
  else if (in_D0w == 1) {
    puVar1 = &UNK_000086ba;
    puVar2 = &LAB_0000f848;
  }
  else {
    puVar1 = &UNK_000086d0;
    puVar2 = (undefined1 *)0xfc48;
  }
  draw_hud_panel(puVar2,puVar1);
  run_hud_panel_post_hook();
  draw_hud_panel();
  return;
}


