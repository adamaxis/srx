// Ghidra headless export
// Entry: 0x00015118
// Name: FUN_00015118
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00015118(void)

{
  short sVar1;
  undefined2 *in_A1;
  undefined2 *puVar2;
  
  sVar1 = 0x7f;
  puVar2 = in_A1;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  in_A1[4] = 0xfff4;
  in_A1[7] = 1;
  in_A1[0xe] = 0x1000;
  in_A1[0x16] = 0x4000;
  *(byte *)(in_A1 + 8) = *(byte *)(in_A1 + 8) | 1;
  *(undefined1 **)(in_A1 + 0x1d) = &LAB_00014e0a;
  return;
}


