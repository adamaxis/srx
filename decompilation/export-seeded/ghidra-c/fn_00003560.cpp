// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003560


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
    puVar1 = (undefined *)0x86ba;
    puVar2 = &LAB_0000f848;
  }
  else {
    puVar1 = (undefined *)0x86d0;
    puVar2 = &LAB_0000fc48;
  }
  FUN_00003648(puVar2,puVar1);
  FUN_00003602();
  FUN_00003648();
  return;
}


