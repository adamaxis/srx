// Ghidra headless export
// Entry: 0x0000BBE4
// Name: commit_entity_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void commit_entity_position(void)

{
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  undefined2 *in_A0;
  
  *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xfe;
  *in_A0 = unaff_D5w;
  in_A0[2] = unaff_D6w;
  return;
}


