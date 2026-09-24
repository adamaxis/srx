// Ghidra headless export
// Entry: 0x000014CE
// Name: FUN_000014ce
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000014ce(void)

{
  short in_D0w;
  short unaff_D7w;
  undefined1 *puVar1;
  undefined1 *in_A1;
  
  puVar1 = &DAT_00200001 + (short)(in_D0w * 2);
  do {
    *in_A1 = *puVar1;
    puVar1 = puVar1 + 2;
    unaff_D7w = unaff_D7w + -1;
    in_A1 = in_A1 + 1;
  } while (unaff_D7w != -1);
  return;
}


