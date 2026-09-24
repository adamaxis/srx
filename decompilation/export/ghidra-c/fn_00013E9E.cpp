// Ghidra headless export
// Entry: 0x00013E9E
// Name: reposition_entity_at_boundary
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void reposition_entity_at_boundary(void)

{
  byte bVar1;
  ushort uVar2;
  short sVar3;
  short sVar4;
  int in_A0;
  
  bVar1 = *(byte *)(in_A0 + 0xb0);
  *(byte *)(in_A0 + 0xb0) = bVar1 | 0x40;
  if ((bVar1 & 0x40) != 0) {
    return;
  }
  sVar3 = 0x2580;
  if ((ushort)(DAT_00ffddce >> 1) < (ushort)(DAT_00ffddc6 + 0xa00U)) {
    sVar3 = -0x2580;
  }
  sVar4 = 0x2580;
  uVar2 = DAT_00ffddc8 + 0xa00;
  if (DAT_00ffddd0 >> 1 < uVar2) {
    sVar4 = -0x2580;
  }
  *(ushort *)(in_A0 + 0x96) = DAT_00ffddc6 + 0xa00U + sVar3;
  *(ushort *)(in_A0 + 0x98) = uVar2 + sVar4;
  return;
}


