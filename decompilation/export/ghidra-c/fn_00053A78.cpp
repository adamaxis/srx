// Ghidra headless export
// Entry: 0x00053A78
// Name: FUN_00053a78
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00053a78(void)

{
  int iVar1;
  short unaff_D6w;
  
  DAT_00fff10c = 0;
  iVar1 = -1;
  do {
    iVar1 = iVar1 + 1;
    if (0x16 < (short)iVar1) {
      return;
    }
    _DAT_00ffdd64 = iVar1;
    (**(code **)((int)&PTR_DAT_0015fd5c + CONCAT22((short)((uint)iVar1 >> 0x10),(short)iVar1 << 2)))
              ();
    iVar1 = _DAT_00ffdd64;
  } while (-1 < unaff_D6w);
  return;
}


