// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00056116


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00056116(void)

{
  char cVar1;
  undefined2 extraout_D0u;
  undefined2 extraout_D0u_00;
  undefined2 extraout_D0u_01;
  undefined2 uVar2;
  undefined4 in_D0;
  
  uVar2 = (undefined2)((uint)in_D0 >> 0x10);
  if (DAT_00fff0a3 < '\0') {
    FUN_00019bf6();
    FUN_000562de();
    FUN_00000dba();
    cVar1 = DAT_00fff0dd;
    FUN_00055f44();
    FUN_000562de();
    FUN_00000dba();
    if ((DAT_00fff0dd < cVar1) && (FUN_0001989a(), uVar2 = extraout_D0u, DAT_00fff0b1 < '\0'))
    goto LAB_000561e2;
    FUN_00017a5e();
    if (DAT_00fff0a0 < '\x01') {
      DAT_00ffdf84 = 0;
      uVar2 = extraout_D0u_00;
    }
    else {
      FUN_0001a908();
      uVar2 = extraout_D0u_01;
    }
  }
  else if (DAT_00fff0a3 < '\x01') {
    if (DAT_00fff0a0 != '\x01') {
      DAT_00fff0a3 = 0xff;
      return 2;
    }
    goto LAB_000561e2;
  }
  DAT_00fff0a3 = '\0';
LAB_000561e2:
  return CONCAT22(uVar2,0xffff);
}


