// Ghidra headless export
// Entry: 0x0000195C
// Name: sample_controller_input
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000019F2 FUN_000019f2


void sample_controller_input(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = FUN_000019f2();
  uVar1 = DAT_00ffdd20;
  DAT_00ffdd20 = uVar2 & 0xff;
  DAT_00ffdd22 = DAT_00ffdd20 | ((DAT_00ffdd20 ^ uVar1) & DAT_00ffdd20) << 8;
  DAT_00ffdd04 = *(undefined2 *)((int)&PTR_DAT_000019d2 + (int)(short)((uVar2 & 0xf) << 1));
  return;
}


