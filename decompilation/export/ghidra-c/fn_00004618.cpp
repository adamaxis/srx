// Ghidra headless export
// Entry: 0x00004618
// Name: update_entity_render_pass
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004658 FUN_00004658
//   0x00005A56 project_entity_screen_position
//   0x00005FDC advance_entity_animation_accumulator
//   0x0000600E FUN_0000600e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void update_entity_render_pass(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  _DAT_00ffde88 = 1;
  sVar1 = 0xf;
  do {
    if ((*(byte *)(puVar2 + 8) & 1) != 0) {
      project_entity_screen_position();
      advance_entity_animation_accumulator();
      FUN_0000600e();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00004658();
  _DAT_00ffde88 = 0;
  return;
}


