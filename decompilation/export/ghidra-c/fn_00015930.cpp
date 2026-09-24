// Ghidra headless export
// Entry: 0x00015930
// Name: FUN_00015930
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000049A0 FUN_000049a0
//   0x0001582A thunk_FUN_0001a2cc
//   0x000530DE FUN_000530de


void FUN_00015930(void)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short extraout_D1w_00;
  int extraout_A1;
  undefined1 in_CF;
  
  uVar1 = thunk_FUN_0001a2cc();
  if (!(bool)in_CF) {
    *(undefined1 *)(extraout_A1 + 0xb2) = 2;
    *(undefined2 *)(extraout_A1 + 0x32) = uVar1;
    *(undefined2 *)(extraout_A1 + 0x2a) = extraout_D1w;
    *(undefined1 *)(extraout_A1 + 2) = 0;
    *(undefined1 *)(extraout_A1 + 3) = 0;
    sVar2 = 2;
    if (u_44178_0000494a[0] != L'\0') {
      FUN_000530de();
      sVar2 = extraout_D1w_00;
    }
    (**(code **)((int)&PTR_FUN_00004964 + (int)(short)(sVar2 << 2)))();
  }
  return;
}


