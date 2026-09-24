// Ghidra headless export
// Entry: 0x00001BEA
// Name: FUN_00001bea
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00001bea(void)

{
  if (((DAT_00ffdd22 & 0x7f00) != 0) && (DAT_00fff101 < 8)) {
    *(byte *)((int)&DAT_00ffcb40 + (int)(short)DAT_00fff101) = (byte)(DAT_00ffdd22 >> 8) & 0x7f;
    DAT_00fff101 = DAT_00fff101 + '\x01';
  }
  return;
}


