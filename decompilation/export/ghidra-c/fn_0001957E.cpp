// Ghidra headless export
// Entry: 0x0001957E
// Name: clear_ready_menu_visual_slots
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001952C FUN_0001952c


void clear_ready_menu_visual_slots(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *extraout_A1;
  
  puVar2 = &DAT_00fffbbe;
  sVar1 = 4;
  do {
    if (-1 < (char)puVar2[sVar1 + 0x1a]) {
      FUN_0001952c();
      puVar2 = extraout_A1;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


