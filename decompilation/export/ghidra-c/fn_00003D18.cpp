// Ghidra headless export
// Entry: 0x00003D18
// Name: FUN_00003d18
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D4A FUN_00003d4a
//   0x00004000 FUN_00004000
//   0x000041B6 FUN_000041b6


void FUN_00003d18(void)

{
  short sVar1;
  short sVar2;
  ushort unaff_D3w;
  short unaff_D4w;
  ushort uVar3;
  short unaff_D5w;
  
  FUN_000041b6();
  sVar1 = FUN_00004000();
  sVar2 = 0x1e;
  uVar3 = unaff_D5w + unaff_D4w;
  do {
    FUN_00003d4a(sVar1,sVar2,unaff_D3w);
    unaff_D3w = uVar3 & 0xf000;
    sVar1 = sVar1 + 8;
    sVar2 = sVar2 + -1;
    uVar3 = uVar3 & 0xf000;
  } while (sVar2 != -1);
  return;
}


