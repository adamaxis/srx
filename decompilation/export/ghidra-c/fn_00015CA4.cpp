// Ghidra headless export
// Entry: 0x00015CA4
// Name: compute_diagonal_step_cost
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Removing unreachable block (ram,0x00015cba) */

undefined4 compute_diagonal_step_cost(void)

{
  ushort uVar1;
  undefined2 uVar2;
  undefined4 in_D0;
  ushort uVar3;
  undefined4 in_D1;
  short unaff_D2w;
  short unaff_D3w;
  
  uVar1 = (short)in_D1 - unaff_D3w;
  if ((short)uVar1 < 0) {
    uVar1 = -uVar1;
  }
  uVar3 = (short)in_D0 - unaff_D2w;
  if ((short)uVar3 < 0) {
    uVar3 = -uVar3;
  }
  uVar2 = (short)((uint)in_D0 >> 0x10);
  if (uVar3 < uVar1) {
    uVar3 = uVar1;
    uVar2 = (short)((uint)in_D1 >> 0x10);
  }
  return CONCAT22(uVar2,uVar3);
}


