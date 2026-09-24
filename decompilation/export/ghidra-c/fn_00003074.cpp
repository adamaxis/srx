// Ghidra headless export
// Entry: 0x00003074
// Name: FUN_00003074
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C7F8 FUN_0000c7f8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003074(void)

{
  short sVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  if (_DAT_00ffe108 == 0) {
    FUN_0000c7f8();
  }
  DAT_00fffbf8 = (undefined1)DAT_00ffddc4;
  sVar1 = FUN_00003032();
  sVar2 = 0x2f;
  puVar3 = *(undefined2 **)((int)&PTR_DAT_00003232 + (int)(short)(sVar1 << 2));
  puVar4 = &DAT_00ffddc6;
  do {
    *puVar4 = *puVar3;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar2 != -1);
  DAT_00ffde22 = DAT_00ffddde;
  if ((_DAT_00fffc0e != 0) && (_DAT_00ffe10e == 0)) {
    DAT_00ffde08 = DAT_00ffe10a;
    DAT_00ffde0c = DAT_00ffe10a;
    DAT_00ffde10 = DAT_00ffe10a;
    DAT_00ffde0a = DAT_00ffe10c;
    DAT_00ffde0e = DAT_00ffe10c;
    DAT_00ffde12 = DAT_00ffe10c;
    DAT_00ffddc6 = DAT_00ffe10a - 0xa00;
    if (DAT_00ffe10a < 0xa00) {
      DAT_00ffddc6 = 0;
    }
    DAT_00ffddc8 = DAT_00ffe10c - 0x700;
    if (DAT_00ffe10c < 0x700) {
      DAT_00ffddc8 = 0;
    }
  }
  _DAT_00ffe10e = 0;
  sVar1 = 0xe0f;
  puVar3 = DAT_00ffddda;
  puVar4 = &DAT_00ff2700;
  do {
    *puVar4 = *puVar3;
    sVar1 = sVar1 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar1 != -1);
  sVar1 = 0xe0f;
  puVar3 = DAT_00ffdde2;
  puVar4 = &DAT_00ff4700;
  do {
    *puVar4 = *puVar3;
    sVar1 = sVar1 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar1 != -1);
  return;
}


