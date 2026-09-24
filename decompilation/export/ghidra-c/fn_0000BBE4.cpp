// Ghidra headless export
// Entry: 0x0000BBE4
// Name: FUN_0000bbe4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000bbe4(void)

{
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  undefined2 *in_A0;
  
  *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xfe;
  *in_A0 = unaff_D5w;
  in_A0[2] = unaff_D6w;
  return;
}


