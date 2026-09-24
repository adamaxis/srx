// Ghidra headless export
// Entry: 0x000061CC
// Name: FUN_000061cc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000061cc(void)

{
  undefined4 *puVar1;
  short in_D0w;
  int unaff_A2;
  
  puVar1 = DAT_00ffdf32;
  *DAT_00ffdf32 = *(undefined4 *)(unaff_A2 + 2);
  *(undefined2 *)(puVar1 + 1) = *(undefined2 *)(unaff_A2 + 6);
  *(short *)((int)puVar1 + 6) = in_D0w << 5;
  DAT_00ffdf32 = (undefined4 *)CONCAT31(DAT_00ffdf32._0_3_,(char)DAT_00ffdf32 + '\b');
  DAT_00ffdf30 = DAT_00ffdf30 + 1;
  return;
}


