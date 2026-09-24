// Ghidra headless export
// Entry: 0x0005670E
// Name: refresh_mode_palette_data
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void refresh_mode_palette_data(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  sVar1 = 0x17;
  puVar2 = &DAT_0018ac56;
  puVar3 = &DAT_00ffd89e;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


