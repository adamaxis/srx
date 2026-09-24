// Ghidra headless export
// Entry: 0x00001D7A
// Name: FUN_00001d7a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000012F6 FUN_000012f6
//   0x00001406 FUN_00001406
//   0x00001CFC FUN_00001cfc
//   0x0000C0C2 FUN_0000c0c2


void FUN_00001d7a(void)

{
  DAT_00ffdd08 = 0;
  DAT_00ffdd0c = 0;
  DAT_00ffdd0a = 0;
  DAT_00ffdd0e = 0;
  FUN_00001406();
  FUN_00001cfc();
  FUN_0000c0c2();
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  FUN_000012f6();
  return;
}


