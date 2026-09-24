// Ghidra headless export
// Entry: 0x00001502
// Name: FUN_00001502
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00001502(void)

{
  undefined4 in_D0;
  short unaff_D7w;
  undefined1 *puVar1;
  undefined1 *puVar2;
  
  puVar2 = &DAT_00200001 + (short)((short)in_D0 * 2);
  puVar1 = &DAT_00200001 + (short)((short)((uint)in_D0 >> 0x10) * 2);
  do {
    *puVar2 = *puVar1;
    puVar1 = puVar1 + 2;
    puVar2 = puVar2 + 2;
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


