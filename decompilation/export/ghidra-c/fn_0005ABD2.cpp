// Ghidra headless export
// Entry: 0x0005ABD2
// Name: FUN_0005abd2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005AC0E FUN_0005ac0e
//   0x0005AC36 FUN_0005ac36
//   0x0005AC7E FUN_0005ac7e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005abd2(void)

{
  DAT_00ffdd02 = 1;
  FUN_0005ac7e();
  FUN_0005ac0e();
  FUN_0005ac36();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


