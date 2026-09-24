// Ghidra headless export
// Entry: 0x0005AB96
// Name: FUN_0005ab96
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005B2E2 FUN_0005b2e2


ushort FUN_0005ab96(void)

{
  ushort uVar1;
  
  uVar1 = DAT_00ffdf88 - DAT_00fff11e;
  if ((0xb3 < uVar1) && (uVar1 = uVar1 & 0xb, uVar1 == 0)) {
    FUN_0005b2e2();
    FUN_0005b2e2();
    FUN_0005b2e2();
    uVar1 = FUN_0005b2e2();
  }
  return uVar1;
}


