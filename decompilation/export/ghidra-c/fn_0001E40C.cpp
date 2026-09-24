// Ghidra headless export
// Entry: 0x0001E40C
// Name: FUN_0001e40c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e40c(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort extraout_D1w;
  short sVar3;
  short sVar4;
  short *psVar5;
  short *psVar6;
  bool bVar7;
  
  psVar6 = &DAT_00ffdd50;
  sVar3 = 0x500;
  sVar4 = 0x500;
  if ((ushort)(DAT_00ffddce >> 1) < (ushort)(DAT_00ffddc6 + 0xa00U)) {
    sVar3 = -0x500;
  }
  uVar1 = DAT_00ffddc8 + 0x700;
  if (DAT_00ffddd0 >> 1 < uVar1) {
    sVar4 = -0x500;
  }
  sVar3 = (sVar3 + DAT_00ffddc6 + 0xa00U & 0xfe00) + 0x100;
  uVar2 = uVar1 & 0xfe00;
  bVar7 = (ushort)(uVar2 + 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar6 = &DAT_00ffdd54;
    DAT_00ffdd50 = sVar3;
    _DAT_00ffdd52 = uVar2 + 0x100;
  }
  bVar7 = (ushort)(uVar2 + 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = sVar3;
    psVar6 = psVar6 + 2;
    *psVar5 = uVar2 + 0x300;
  }
  bVar7 = (ushort)(uVar2 + 0x500) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = sVar3;
    psVar6 = psVar6 + 2;
    *psVar5 = uVar2 + 0x500;
  }
  bVar7 = (ushort)(uVar2 - 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = sVar3;
    psVar6 = psVar6 + 2;
    *psVar5 = uVar2 - 0x100;
  }
  bVar7 = (ushort)(uVar2 - 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = sVar3;
    psVar6 = psVar6 + 2;
    *psVar5 = uVar2 - 0x300;
  }
  uVar2 = extraout_D1w & 0xfe00;
  sVar3 = (sVar4 + uVar1 & 0xfe00) + 0x100;
  bVar7 = sVar3 == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = uVar2 + 0x100;
    psVar6 = psVar6 + 2;
    *psVar5 = sVar3;
  }
  bVar7 = (ushort)(uVar2 + 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = uVar2 + 0x300;
    psVar6 = psVar6 + 2;
    *psVar5 = sVar3;
  }
  bVar7 = (ushort)(uVar2 + 0x500) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = uVar2 + 0x500;
    psVar6 = psVar6 + 2;
    *psVar5 = sVar3;
  }
  bVar7 = (ushort)(uVar2 - 0x100) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = uVar2 - 0x100;
    psVar6 = psVar6 + 2;
    *psVar5 = sVar3;
  }
  bVar7 = (ushort)(uVar2 - 0x300) == 0;
  FUN_0001e41e();
  if (bVar7) {
    psVar5 = psVar6 + 1;
    *psVar6 = uVar2 - 0x300;
    psVar6 = psVar6 + 2;
    *psVar5 = sVar3;
  }
  *psVar6 = 0;
  return;
}


