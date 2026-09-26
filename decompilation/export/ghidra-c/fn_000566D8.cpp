// Ghidra headless export
// Entry: 0x000566D8
// Name: copy_mode_resource_metadata
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_mode_resource_metadata(void)

{
  short sVar1;
  undefined4 *in_A0;
  undefined4 *puVar2;
  
  sVar1 = 7;
  puVar2 = &DAT_00ffd87e;
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


