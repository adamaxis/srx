// Ghidra headless export
// Entry: 0x0005787C
// Name: FUN_0005787c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005787c(void)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = (uint)DAT_00fff0d6;
  if (DAT_00fff0d0 < '\x01') {
    _DAT_00fff0be = (uVar2 * uVar2 * uVar2 & 0xffff) * 0x19;
  }
  else {
    if ((char)(DAT_00fff0d0 + -3) < '\0') {
      iVar1 = 3;
    }
    else {
      iVar1 = 2;
    }
    _DAT_00fff0be = (uVar2 * uVar2 * iVar1 & 0xffff) * 100;
  }
  return;
}


