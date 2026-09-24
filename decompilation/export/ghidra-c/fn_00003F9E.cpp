// Ghidra headless export
// Entry: 0x00003F9E
// Name: FUN_00003f9e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003FD8 FUN_00003fd8


undefined4 FUN_00003f9e(void)

{
  int iVar1;
  int extraout_D1;
  short unaff_D3w;
  
  FUN_00003fd8();
  iVar1 = extraout_D1 + *(short *)((int)&DAT_00ff2700 + (int)unaff_D3w) * 0x20;
  return CONCAT22((short)((uint)iVar1 >> 0x10),
                  ((*(ushort *)(DAT_00ffddde + (short)iVar1) & 0x7ff) - 0x201) * 0x20);
}


