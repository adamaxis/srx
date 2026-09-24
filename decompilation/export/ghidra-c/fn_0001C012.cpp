// Ghidra headless export
// Entry: 0x0001C012
// Name: FUN_0001c012
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001c012(void)

{
  short sVar1;
  undefined2 *in_A0;
  undefined2 *puVar2;
  
  sVar1 = 6;
  puVar2 = (undefined2 *)((int)&DAT_00ffd87e + 2);
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


