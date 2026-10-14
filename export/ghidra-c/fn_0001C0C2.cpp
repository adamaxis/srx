// Ghidra headless export
// Entry: 0x0001C0C2
// Name: advance_alternating_profile
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019DE8 run_menu_service_then_prepare


void advance_alternating_profile(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  DAT_00ffe0aa = DAT_00ffe0aa - 1;
  if (DAT_00ffe0aa != 0) {
    puVar2 = &DAT_0001c126;
    if ((DAT_00ffe0aa & 1) != 0) {
      puVar2 = &DAT_0001c128;
    }
    sVar1 = 6;
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
  run_menu_service_then_prepare();
  return;
}


