// Ghidra headless export
// Entry: 0x00014C56
// Name: project_heading_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined2 project_heading_position(void)

{
  int in_A0;
  
  return *(undefined2 *)(&DAT_0000b2f8 + (short)((*(ushort *)(in_A0 + 0x2e) >> 0xd) << 2));
}


