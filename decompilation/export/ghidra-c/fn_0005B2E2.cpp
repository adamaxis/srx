// Ghidra headless export
// Entry: 0x0005B2E2
// Name: FUN_0005b2e2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0005b2e2(void)

{
  short in_D0w;
  ushort uVar1;
  short sVar2;
  ushort *puVar3;
  
  sVar2 = 0xe;
  puVar3 = (ushort *)((int)&DAT_00ffd87e + (int)(short)(in_D0w * 0x20 + 2));
  do {
    uVar1 = *puVar3;
    if ((uVar1 & 0xe) != 0) {
      uVar1 = uVar1 - 2;
    }
    if ((uVar1 & 0xe0) != 0) {
      uVar1 = uVar1 - 0x20;
    }
    if ((uVar1 & 0xe00) != 0) {
      uVar1 = uVar1 - 0x200;
    }
    *puVar3 = uVar1;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
  } while (sVar2 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


