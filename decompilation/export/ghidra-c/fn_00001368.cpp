// Ghidra headless export
// Entry: 0x00001368
// Name: FUN_00001368
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00001368(void)

{
  short sVar1;
  undefined4 *in_A0;
  undefined4 *puVar2;
  
  sVar1 = 0x1f;
  puVar2 = &DAT_00ffd8fe;
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


