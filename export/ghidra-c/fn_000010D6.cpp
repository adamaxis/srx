// Ghidra headless export
// Entry: 0x000010D6
// Name: step_palette_toward_target
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void step_palette_toward_target(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  short sVar5;
  ushort *puVar6;
  ushort *puVar7;
  
  sVar5 = 0x3f;
  puVar6 = (ushort *)&DAT_00ffd87e;
  puVar7 = (ushort *)&DAT_00ffd8fe;
  do {
    uVar1 = *puVar6;
    uVar2 = uVar1 & 0xe;
    if (uVar2 != (*puVar7 & 0xe)) {
      uVar2 = uVar2 + 2;
    }
    uVar3 = uVar1 & 0xe0;
    if (uVar3 != (*puVar7 & 0xe0)) {
      uVar3 = uVar3 + 0x20;
    }
    uVar4 = uVar1 & 0xe00;
    if (uVar4 != (*puVar7 & 0xe00)) {
      uVar4 = uVar4 + 0x200;
    }
    *puVar6 = ((uVar1 & 0xeee0 | uVar2) & 0xee0e | uVar3) & 0xe0ee | uVar4;
    sVar5 = sVar5 + -1;
    puVar6 = puVar6 + 1;
    puVar7 = puVar7 + 1;
  } while (sVar5 != -1);
  return;
}


