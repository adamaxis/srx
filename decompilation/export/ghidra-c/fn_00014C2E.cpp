// Ghidra headless export
// Entry: 0x00014C2E
// Name: FUN_00014c2e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00014c2e(void)

{
  ushort *in_A0;
  
  *(undefined1 *)((int)in_A0 + 0xb1) = 0;
  if (*in_A0 <= in_A0[0x4b]) {
    *(byte *)((int)in_A0 + 0xb1) = *(byte *)((int)in_A0 + 0xb1) | 1;
  }
  if (in_A0[2] <= in_A0[0x4c]) {
    *(byte *)((int)in_A0 + 0xb1) = *(byte *)((int)in_A0 + 0xb1) | 2;
  }
  return;
}


