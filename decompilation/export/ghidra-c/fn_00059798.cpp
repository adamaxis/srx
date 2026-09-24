// Ghidra headless export
// Entry: 0x00059798
// Name: FUN_00059798
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000597EE FUN_000597ee


uint FUN_00059798(void)

{
  uint uVar1;
  
  FUN_000597ee();
  if (-1 < DAT_00fffc0c) {
    FUN_000597ee();
  }
  uVar1 = (uint)DAT_00fffc0d;
  if (-1 < (char)DAT_00fffc0d) {
    uVar1 = FUN_000597ee();
  }
  return uVar1;
}


