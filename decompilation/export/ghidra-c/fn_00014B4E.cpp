// Ghidra headless export
// Entry: 0x00014B4E
// Name: FUN_00014b4e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014b4e(void)

{
  ushort in_D0w;
  ushort unaff_D5w;
  
  _DAT_00ffdd2c = ((in_D0w >> 6) << 9 | in_D0w >> 0xd) + 0x100;
  DAT_00ffdd2a = (in_D0w & 0x3f) * 0x200 + 0x100;
  if (((unaff_D5w & 0xffc0) != (in_D0w & 0xffc0)) && ((in_D0w & 0xffc0) <= (unaff_D5w & 0xffc0))) {
    return;
  }
  return;
}


