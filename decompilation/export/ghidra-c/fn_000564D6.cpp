// Ghidra headless export
// Entry: 0x000564D6
// Name: convert_intro_routine_0x000564d6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_intro_routine_0x000564d6(void)

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


