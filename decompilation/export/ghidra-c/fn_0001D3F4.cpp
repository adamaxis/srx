// Ghidra headless export
// Entry: 0x0001D3F4
// Name: FUN_0001d3f4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound


uint FUN_0001d3f4(void)

{
  int iVar1;
  char cVar2;
  short in_D0w;
  short sVar4;
  uint uVar3;
  byte bVar5;
  
  iVar1 = *(int *)(&DAT_000f1c5e + (short)(in_D0w << 2));
  sVar4 = sample_source_random_bound();
  bVar5 = *(byte *)(iVar1 + 1 + (int)sVar4);
  uVar3 = 0;
  sVar4 = 7;
  do {
    uVar3 = uVar3 << 4;
    cVar2 = (char)bVar5 >> 7;
    bVar5 = bVar5 << 1 | -cVar2;
    if (-cVar2 != 0) {
      uVar3 = uVar3 | 0xf;
    }
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  return uVar3;
}


