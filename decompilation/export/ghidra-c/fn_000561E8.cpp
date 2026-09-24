// Ghidra headless export
// Entry: 0x000561E8
// Name: FUN_000561e8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA FUN_00000dba
//   0x00017D72 FUN_00017d72
//   0x00017D90 FUN_00017d90
//   0x00019B40 FUN_00019b40
//   0x00019BF6 FUN_00019bf6
//   0x00053216 FUN_00053216
//   0x00056116 FUN_00056116
//   0x000562D4 FUN_000562d4


undefined4 FUN_000561e8(void)

{
  char cVar1;
  undefined2 extraout_D0u;
  undefined2 extraout_D0u_00;
  undefined2 extraout_D0u_01;
  undefined2 uVar2;
  undefined2 extraout_D0u_02;
  undefined4 uVar3;
  short sVar4;
  int extraout_A0;
  
  FUN_00053216();
  sVar4 = DAT_00fff118 * 0x14;
  if (*(char *)(extraout_A0 + 0x10 + (int)sVar4) != (&DAT_00ff1110)[sVar4]) {
    (&DAT_00ff1110)[sVar4] = (&DAT_00ff1110)[sVar4] + '\x01';
  }
  cVar1 = (&DAT_00ff1113)[sVar4];
  if (-1 < (char)(cVar1 + -2)) {
    if ((char)(cVar1 + -5) < '\0') goto LAB_00056268;
    if ((char)(cVar1 + -7) < '\0') {
LAB_000560aa:
      if (DAT_00fff0e6 < '\0') {
        DAT_00fff0e7 = DAT_00fff0e7 + -1;
      }
      else {
        FUN_00019bf6();
        FUN_000562d4();
        FUN_00000dba();
        DAT_00fff0dd = DAT_00fff0dd + -1;
        if (-1 < DAT_00fff0dd) {
          DAT_00fff0e7 = '\n' - DAT_00fff0dd;
          DAT_00fff0e6 = 0xff;
          return 3;
        }
      }
      return 0xffffffff;
    }
    if ((char)(cVar1 + -0xb) < '\0') {
      if (DAT_00fff0a0 < '\x01') {
        uVar3 = FUN_00056116();
        return uVar3;
      }
      goto LAB_000560aa;
    }
    if ((char)(cVar1 + -0x11) < '\0') {
      if (DAT_00fff0a0 < '\x01') {
        uVar3 = FUN_00056116();
        return uVar3;
      }
LAB_00056268:
      FUN_00019bf6();
      FUN_000562d4();
      FUN_00000dba();
      sVar4 = FUN_00019b40();
      if (sVar4 == 8) {
        FUN_00017d72();
        FUN_00017d90();
      }
      uVar3 = 0;
      DAT_00fff0a4 = DAT_00fff0dd;
      if (DAT_00fff0dd < '\x01') {
        uVar3 = 0xffff;
        DAT_00fff0a4 = '\x01';
      }
      return uVar3;
    }
    if (-1 < (char)(cVar1 + -0x15)) goto LAB_00056268;
  }
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
  else {
    uVar2 = extraout_D0u_02;
    if (DAT_00fff0a3 < '\x01') {
      if (DAT_00fff0a0 != '\x01') {
        DAT_00fff0a3 = 0xff;
        return 2;
      }
      goto LAB_000561e2;
    }
  }
  DAT_00fff0a3 = '\0';
LAB_000561e2:
  return CONCAT22(uVar2,0xffff);
}


