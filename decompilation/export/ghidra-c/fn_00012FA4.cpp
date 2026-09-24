// Ghidra headless export
// Entry: 0x00012FA4
// Name: FUN_00012fa4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x0000C0DA FUN_0000c0da
//   0x00010660 FUN_00010660


void FUN_00012fa4(void)

{
  ushort uVar1;
  int extraout_A0;
  char cVar2;
  bool bVar3;
  char cVar4;
  
  FUN_0000c0da();
  uVar1 = (ushort)*(byte *)(extraout_A0 + 0x57);
  cVar4 = SBORROW2(uVar1,1);
  cVar2 = (short)(uVar1 - 1) < 0;
  bVar3 = uVar1 == 1;
  FUN_00010660();
  if (!bVar3 && cVar4 == cVar2) {
    FUN_00001232();
  }
  return;
}


