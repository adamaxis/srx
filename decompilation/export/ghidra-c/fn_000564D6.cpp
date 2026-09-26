// Ghidra headless export
// Entry: 0x000564D6
// Name: reduce_menu_progression_timing
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


// Direct disassembly halves D1, clamps the delta to one, subtracts it from
// record +0x06, and clamps the resulting timing byte to two. The typed host
// conversion is in ghidra_conversion/menu_selection_timing_conversion.
void reduce_menu_progression_timing(void)

{
  char *pcVar1;
  short sVar2;
  ushort in_D1w;
  ushort uVar3;
  
  sVar2 = DAT_00fff118 * 0x14;
  uVar3 = in_D1w >> 1;
  if (uVar3 == 0 || (short)in_D1w < 0) {
    uVar3 = 1;
  }
  pcVar1 = (char *)((int)&DAT_00ff1104 + sVar2 + 2);
  *pcVar1 = *pcVar1 - (char)uVar3;
  if ((char)(*(char *)((int)&DAT_00ff1104 + sVar2 + 2) + -2) < '\0') {
    *(undefined1 *)((int)&DAT_00ff1104 + sVar2 + 2) = 2;
  }
  return;
}

