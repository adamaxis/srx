// Ghidra headless export
// Entry: 0x00001088
// Name: step_palette_toward_black
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void step_palette_toward_black(void)

{
  ushort uVar1;
  ushort uVar2;
  short sVar3;
  ushort *puVar4;
  
  sVar3 = 0x3f;
  puVar4 = (ushort *)&DAT_00ffd87e;
  do {
    uVar2 = *puVar4 & 0xe;
    if (uVar2 != 0) {
      uVar2 = uVar2 - 2;
    }
    uVar2 = *puVar4 & 0xeee0 | uVar2;
    uVar1 = uVar2 & 0xe0;
    if (uVar1 != 0) {
      uVar1 = uVar1 - 0x20;
    }
    uVar1 = uVar2 & 0xee0e | uVar1;
    uVar2 = uVar1 & 0xe00;
    if (uVar2 != 0) {
      uVar2 = uVar2 - 0x200;
    }
    *puVar4 = uVar1 & 0xe0ee | uVar2;
    sVar3 = sVar3 + -1;
    puVar4 = puVar4 + 1;
  } while (sVar3 != -1);
  return;
}


