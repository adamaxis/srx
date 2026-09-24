// Ghidra headless export
// Entry: 0x0000D5DC
// Name: FUN_0000d5dc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216


void FUN_0000d5dc(void)

{
  undefined4 uVar1;
  undefined4 unaff_D5;
  uint uVar2;
  uint uVar3;
  short sVar4;
  
  if ((char)(DAT_00fff0d1 - 4) < '\0') {
    uVar1 = FUN_0000c0da();
    if ((ushort)((ushort)DAT_00fff0d1 + (short)CONCAT31((int3)((uint)uVar1 >> 8),DAT_00fff0d0) * 4)
        == 6) {
      FUN_0000c216();
      FUN_0000c216();
      goto LAB_0000d71e;
    }
  }
  else {
    FUN_0000c0da();
  }
  FUN_0000c216();
LAB_0000d71e:
  uVar2 = CONCAT22((short)((uint)unaff_D5 >> 0x10),*(undefined2 *)(DAT_00fffc04 + 0x90));
  sVar4 = 3;
  uVar3 = 5;
  do {
    if ((uVar2 & 1 << (uVar3 & 0x1f)) != 0) {
      FUN_0000c0da();
      FUN_0000c0da();
      FUN_0000c216();
      FUN_0000c216();
      break;
    }
    uVar3 = (uint)(ushort)((short)uVar3 + 1);
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  sVar4 = 2;
  uVar3 = 9;
  do {
    if ((uVar2 & 1 << (uVar3 & 0x1f)) != 0) {
      FUN_0000c0da();
      FUN_0000c216();
      return;
    }
    uVar3 = (uint)(ushort)((short)uVar3 + 1);
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  return;
}


