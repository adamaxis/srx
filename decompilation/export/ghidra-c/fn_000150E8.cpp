// Ghidra headless export
// Entry: 0x000150E8
// Name: FUN_000150e8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000150e8(void)

{
  short sVar1;
  ushort uVar2;
  ushort in_D1w;
  int in_A1;
  
  uVar2 = in_D1w >> 8 & 0xfffe;
  sVar1 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar2);
  *(short *)(in_A1 + 0x4c) = (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar2) * 0xc >> 4);
  *(short *)(in_A1 + 0x4e) = (short)(sVar1 * 0xc >> 4);
  return;
}


