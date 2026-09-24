// Ghidra headless export
// Entry: 0x000061A0
// Name: FUN_000061a0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000061a0(void)

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


