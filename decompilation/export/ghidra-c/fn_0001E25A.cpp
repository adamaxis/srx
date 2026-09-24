// Ghidra headless export
// Entry: 0x0001E25A
// Name: FUN_0001e25a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000044BC FUN_000044bc
//   0x0001A2CC FUN_0001a2cc
//   0x0001A342 FUN_0001a342
//   0x0001DBAA FUN_0001dbaa


void FUN_0001e25a(void)

{
  short sVar1;
  undefined2 uVar2;
  short sVar3;
  short *extraout_A1;
  short *unaff_A4;
  short *psVar4;
  bool bVar5;
  
  bVar5 = false;
  DAT_00ffdd4c = unaff_A4;
  uVar2 = FUN_0001a342();
  if (!bVar5) {
    bVar5 = false;
    DAT_00ffdd40 = uVar2;
    FUN_0001a2cc();
    if (!bVar5) {
      FUN_000044bc();
      *(undefined1 *)((int)extraout_A1 + 0x4d) = 4;
      *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 0x20;
      psVar4 = DAT_00ffdd4c + 1;
      sVar3 = *DAT_00ffdd4c;
      if (*DAT_00ffdd4c == 0) {
        psVar4 = (short *)&DAT_00ffdd52;
        sVar3 = DAT_00ffdd50;
      }
      DAT_00ffdd4c = psVar4 + 1;
      sVar1 = *psVar4;
      *extraout_A1 = sVar3;
      extraout_A1[2] = sVar1;
      DAT_00ffe177 = DAT_00ffe177 + '\x01';
      *(byte *)(extraout_A1 + 0x4f) = *(byte *)(extraout_A1 + 0x4f) | 0x40;
      extraout_A1[0x4b] = *extraout_A1;
      extraout_A1[0x4c] = extraout_A1[2];
      FUN_0001dbaa();
      return;
    }
  }
  return;
}


