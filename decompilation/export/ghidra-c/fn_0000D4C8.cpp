// Ghidra headless export
// Entry: 0x0000D4C8
// Name: FUN_0000d4c8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00012C5A write_menu_repeated_grid_with_metadata


void FUN_0000d4c8(void)

{
  undefined2 *puVar1;
  
  write_menu_repeated_grid_with_metadata();
  if (DAT_00fffc04[0x1f] == 2) {
    *(byte *)(DAT_00fffc04 + 8) = *(byte *)(DAT_00fffc04 + 8) & 0xfd;
LAB_0000d4f2:
    DAT_00fffc04 = &DAT_00ff0100;
    DAT_00ff0110 = DAT_00ff0110 | 2;
  }
  else {
    do {
      puVar1 = DAT_00fffc04;
      *(byte *)(puVar1 + 8) = *(byte *)(puVar1 + 8) & 0xfd;
      DAT_00fffc04 = puVar1 + 0x80;
      if ((-1 < (short)(puVar1[0x9f] + -3)) || (puVar1[0x9f] == 0)) goto LAB_0000d4f2;
    } while ((*(byte *)(puVar1 + 0x88) & 1) == 0);
    *(byte *)(puVar1 + 0x88) = *(byte *)(puVar1 + 0x88) | 2;
  }
  return;
}


