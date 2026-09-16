// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000BB86


undefined4 FUN_0000bb86(void)

{
  undefined4 *puVar1;
  char cVar2;
  undefined4 in_D0;
  uint uVar3;
  short sVar5;
  undefined4 uVar4;
  int in_A0;
  
  uVar3 = CONCAT22((short)((uint)in_D0 >> 0x10),*(ushort *)(in_A0 + 0x2e) >> 0xd) & 0xffff0006;
  sVar5 = (short)uVar3 << 1;
  puVar1 = *(undefined4 **)((int)&PTR_DAT_0000bbf6 + (int)sVar5);
  cVar2 = *(char *)*puVar1;
  if ((cVar2 != '\0') && (-1 < cVar2)) {
    cVar2 = *(char *)puVar1[1];
    if ((cVar2 != '\0') && (-1 < cVar2)) {
      return CONCAT31((int3)(CONCAT22((short)(uVar3 >> 0x10),sVar5) >> 8),cVar2);
    }
  }
  uVar4 = FUN_0000b732();
  if (((short)uVar4 != 0) && (-1 < (short)uVar4)) {
    return uVar4;
  }
  return uVar4;
}


