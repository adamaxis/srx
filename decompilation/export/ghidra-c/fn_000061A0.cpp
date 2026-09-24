// Ghidra headless export
// Entry: 0x000061A0
// Name: append_entity_sprite_record
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void append_entity_sprite_record(void)

{
  undefined4 *puVar1;
  int in_A0;
  int unaff_A2;
  
  puVar1 = DAT_00ffdf32;
  *DAT_00ffdf32 = *(undefined4 *)(unaff_A2 + 2);
  *(undefined2 *)(puVar1 + 1) = *(undefined2 *)(unaff_A2 + 6);
  *(short *)((int)puVar1 + 6) = *(short *)(in_A0 + 0x2a) << 5;
  DAT_00ffdf32 = (undefined4 *)CONCAT31(DAT_00ffdf32._0_3_,(char)DAT_00ffdf32 + '\b');
  DAT_00ffdf30 = DAT_00ffdf30 + 1;
  return;
}


