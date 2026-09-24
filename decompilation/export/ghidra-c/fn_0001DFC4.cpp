// Ghidra headless export
// Entry: 0x0001DFC4
// Name: FUN_0001dfc4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x0001E0B8 FUN_0001e0b8
//   0x000530D2 FUN_000530d2


void FUN_0001dfc4(void)

{
  ushort uVar1;
  ushort *puVar2;
  ushort *puVar4;
  ushort *puVar5;
  ushort *puVar3;
  
  if ((DAT_00ffe16f != '\0') && (DAT_00ffe11e = DAT_00ffe11e + -1, DAT_00ffe11e < 0)) {
    if (DAT_00ffe16f != '\0') {
      DAT_00ffe16f = '\0';
      FUN_0001e0b8();
    }
    FUN_000530d2();
    return;
  }
  if (DAT_00ffe170 != '\0') {
    return;
  }
  if ((DAT_00ffdf88 & 0x3f) == 0xd) {
    uVar1 = advance_source_prng();
    if (0x7fff < uVar1) {
      return;
    }
    puVar2 = *(ushort **)(DAT_00ffde1e + 0xc);
    do {
      puVar3 = puVar2 + 1;
      uVar1 = *puVar2;
      if ((short)uVar1 < 0) {
        return;
      }
      puVar4 = puVar2 + 2;
      puVar5 = puVar2 + 3;
      puVar2 = puVar2 + 4;
    } while ((((*DAT_00fffc04 >> 4 <= uVar1) || (*puVar3 < *DAT_00fffc04 >> 4)) ||
             (DAT_00fffc04[2] >> 4 <= *puVar4)) || (*puVar5 < DAT_00fffc04[2] >> 4));
    if (DAT_00ffddc4 == 0x14) {
      FUN_0001dbaa();
    }
    else if ((DAT_00ffdf40 == 2) && (DAT_00ffe11e = 0x960, DAT_00ffe16f == '\0')) {
      DAT_00ffe16f = -1;
      FUN_000530de();
      FUN_0001df00();
      FUN_0001e0dc();
    }
    return;
  }
  return;
}


