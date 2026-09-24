// Ghidra headless export
// Entry: 0x00003560
// Name: FUN_00003560
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003602 FUN_00003602
//   0x00003648 FUN_00003648


void FUN_00003560(void)

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
  FUN_00003648(puVar2,puVar1);
  FUN_00003602();
  FUN_00003648();
  return;
}


