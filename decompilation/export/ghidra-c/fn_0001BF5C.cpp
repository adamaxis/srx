// Ghidra headless export
// Entry: 0x0001BF5C
// Name: copy_title_profile
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_title_profile(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 6;
  puVar2 = &DAT_0001c10a;
  puVar3 = (undefined2 *)((int)&DAT_00ffd87e + 2);
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


