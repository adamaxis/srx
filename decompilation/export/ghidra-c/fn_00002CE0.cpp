// Ghidra headless export
// Entry: 0x00002CE0
// Name: FUN_00002ce0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DBAA FUN_0001dbaa
//   0x0001E2EC FUN_0001e2ec


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002ce0(void)

{
  short sVar1;
  short sVar2;
  short extraout_D1w;
  short sVar3;
  
  sVar1 = (short)(char)(DAT_00fffbf9 + -8);
  sVar2 = DAT_00fffc10 << 2;
  sVar3 = (short)(char)(&DAT_00002e72)[(int)sVar1 + (int)sVar2];
  DAT_00fffc10 = sVar3;
  if ((sVar3 == 0x13) && ((DAT_00fffc1e & 0x100) == 0)) {
    _DAT_00ffdd2c = &UNK_00053ee2;
    FUN_0001e2ec();
    DAT_00ffe120 = &UNK_0001e552;
    sVar1 = FUN_0001dbaa();
    sVar2 = extraout_D1w;
  }
  DAT_00ffddc4 = *(short *)((int)&PTR_DAT_00002e42 + (int)(short)(sVar3 * 2));
  sVar2 = CONCAT11((char)((ushort)sVar1 >> 8),(&DAT_00002ed2)[(int)sVar1 + (int)sVar2]) << 2;
  if (DAT_00ffddc4 < 0x2c) {
    sVar1 = DAT_00ffddc4 + -6;
  }
  else {
    sVar1 = DAT_00ffddc4 + -0x28;
  }
  DAT_00ffe10a = *(undefined2 *)(&DAT_00002d92 + (int)sVar2 + (int)(short)(sVar1 << 4));
  DAT_00ffe10c = *(undefined2 *)(&UNK_00002d94 + (int)sVar2 + (int)(short)(sVar1 << 4));
  return;
}


