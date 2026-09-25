// Ghidra headless export
// Entry: 0x0001047E
// Name: refresh_ready_entity_animations
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00055F02 FUN_00055f02


void refresh_ready_entity_animations(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 2;
  do {
    if (((*(byte *)(puVar2 + 8) & 1) != 0) && (*(char *)((int)puVar2 + 0x9b) < '\x02')) {
      FUN_00055f02();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x80;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


