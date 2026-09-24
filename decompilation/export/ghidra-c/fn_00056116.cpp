// Ghidra headless export
// Entry: 0x00056116
// Name: FUN_00056116
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00017A5E FUN_00017a5e
//   0x0001989A FUN_0001989a
//   0x00019BF6 FUN_00019bf6
//   0x0001A908 FUN_0001a908
//   0x00055F44 FUN_00055f44
//   0x000562DE FUN_000562de


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
    count_thresholded_random_rolls();
    cVar1 = DAT_00fff0dd;
    FUN_00055f44();
    FUN_000562de();
    count_thresholded_random_rolls();
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


