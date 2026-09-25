// Ghidra headless export
// Entry: 0x000060FA
// Name: append_entity_render_sprite
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003E86 expand_packed_nibbles
//   0x00003EFE FUN_00003efe


void append_entity_render_sprite(void)

{
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int unaff_A2;
  
  if ((*(byte *)(in_A0 + 0x11) & 0x80) != 0) {
    if (*(short *)(in_A0 + 0xc) == 0) {
      return;
    }
    FUN_00003efe();
    in_A0 = extraout_A0;
  }
  if ((*(byte *)(in_A0 + 0x11) & 0x20) != 0) {
    expand_packed_nibbles();
    in_A0 = extraout_A0_00;
  }
  *(undefined4 *)(in_A0 + 0x22) = *(undefined4 *)(unaff_A2 + 2);
  *(undefined2 *)(in_A0 + 0x28) = *(undefined2 *)(unaff_A2 + 6);
  *DAT_00ffdf2c = in_A0;
  DAT_00ffdf2c = (int *)CONCAT31(DAT_00ffdf2c._0_3_,(char)DAT_00ffdf2c + '\x04');
  DAT_00ffdf2a = DAT_00ffdf2a + 1;
  return;
}


