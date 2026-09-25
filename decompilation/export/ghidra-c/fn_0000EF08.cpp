// Ghidra headless export
// Entry: 0x0000EF08
// Name: FUN_0000ef08
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003674 FUN_00003674
//   0x000037EC FUN_000037ec
//   0x0000C0DA render_encoded_title_text
//   0x0000C3E6 write_menu_grid_with_metadata
//   0x0000F312 wait_for_controller_event


void FUN_0000ef08(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  undefined2 *extraout_A0_00;
  
  write_menu_grid_with_metadata();
  puVar2 = &DAT_00ff0100;
  sVar1 = 2;
  do {
    if ((*(byte *)(puVar2 + 8) & 1) != 0) {
      if (((short)(*(byte *)(puVar2 + 0x26) - 5) < 0) && ((short)(*(byte *)(puVar2 + 0x26) - 3) < 0)
         ) {
        FUN_000037ec();
        render_encoded_title_text();
        render_encoded_title_text();
        puVar2 = extraout_A0;
      }
      else {
        FUN_00003674();
        FUN_000037ec();
        *(byte *)(extraout_A0_00 + 8) = *(byte *)(extraout_A0_00 + 8) & 0xef;
        puVar2 = extraout_A0_00;
      }
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  render_encoded_title_text();
  wait_for_controller_event();
  return;
}


