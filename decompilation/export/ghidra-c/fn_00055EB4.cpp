// Ghidra headless export
// Entry: 0x00055EB4
// Name: subtract_armor_secondary_modifier
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 subtract_armor_secondary_modifier(void)

{
  undefined1 uVar1;
  int in_A1;
  
  uVar1 = 0;
  if (*(byte *)(in_A1 + 0x76) != 0) {
    uVar1 = (&DAT_000d4d76)[(short)((*(byte *)(in_A1 + 0x76) - 1) * 0x30)];
  }
  return uVar1;
}


