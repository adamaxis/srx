// Ghidra headless export
// Entry: 0x0001DD94
// Name: FUN_0001dd94
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016ECA FUN_00016eca
//   0x0001DBF0 FUN_0001dbf0
//   0x0001DC4C FUN_0001dc4c
//   0x0001DEA0 FUN_0001dea0
//   0x0001DF1A FUN_0001df1a
//   0x0001DFC4 advance_world_effect_trigger
//   0x0001E166 FUN_0001e166
//   0x0001E2EC FUN_0001e2ec
//   0x00053126 FUN_00053126


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001dd94(void)

{
  int iVar1;
  
  if (((DAT_00ffe186 == '\x02') && (DAT_00fff13e._2_2_ < 0)) && (DAT_00ff044c < '\n')) {
    DAT_00fff13e._2_2_ = 0;
    DAT_00ffe186 = '\x03';
  }
  if ((DAT_00ffe166 != 0) && (DAT_00ffe166 = DAT_00ffe166 + -1, DAT_00ffe166 == 0)) {
    FUN_00053126();
    _DAT_00ffdd2c = FUN_0000c64c;
    FUN_0001e2ec();
  }
  if (DAT_00ffe11c != 0) {
    DAT_00ffe11c = DAT_00ffe11c + -1;
  }
  if (_DAT_00ffdf84 != 0) {
    if (((DAT_00ffe186 == '\0') && (DAT_00ffe16f == '\0')) &&
       ((DAT_00ffddc4 != 0x14 && (DAT_00ffdf86 = DAT_00ffdf86 + -1, DAT_00ffdf86 < 0)))) {
      FUN_0001dbf0();
    }
    else {
      iVar1 = *(int *)(DAT_00fffc04 + 0xa2);
      if (((iVar1 == 0) || ((*(byte *)(iVar1 + 0x10) & 1) == 0)) ||
         (((-1 < *(char *)(DAT_00fffc04 + 0x56) || (*(char *)(DAT_00fffc04 + 0x57) != '\f')) &&
          ((*(byte *)(iVar1 + 0x10) & 0x40) != 0)))) {
        FUN_0001dc4c();
      }
    }
  }
  FUN_0001dea0();
  if (DAT_00ffdf40 == 2) {
    FUN_00016eca();
    advance_world_effect_trigger();
  }
  FUN_0001e166();
  FUN_0001df1a();
  return;
}


