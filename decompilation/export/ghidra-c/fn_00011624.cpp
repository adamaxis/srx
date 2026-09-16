// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00011624


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 FUN_00011624(void)

{
  undefined2 uVar1;
  short sVar2;
  int extraout_A0;
  int iVar3;
  
  uVar1 = FUN_000116c4();
  iVar3 = _DAT_00fffc04;
  if ((-1 < *(char *)(_DAT_00fffc04 + 0x56)) && (*(char *)(_DAT_00fffc04 + 0x56) != '\x7f')) {
    FUN_00011678();
    iVar3 = extraout_A0;
  }
  if (*(char *)(iVar3 + 0x76) != '\0') {
    sVar2 = 7;
    do {
      if ((char)(*(char *)(iVar3 + 0x76) + '+') == *(char *)(iVar3 + 0x66 + (int)sVar2)) {
        FUN_00011678();
        break;
      }
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  }
  FUN_00011136();
  return uVar1;
}


