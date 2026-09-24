// Ghidra headless export
// Entry: 0x00003BAC
// Name: FUN_00003bac
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004018 FUN_00004018
//   0x000040E2 FUN_000040e2
//   0x00015D3C measure_relative_heading_and_distance


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003bac(void)

{
  ushort uVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  undefined1 auStack_6 [2];
  undefined4 uStack_4;
  
  uVar3 = DAT_00ffddc8;
  uVar1 = DAT_00ffddc6;
  uStack_4 = 0x3bca;
  measure_relative_heading_and_distance();
  if (uVar1 < 0x20) {
    _DAT_00ffdf5a = 0;
    return;
  }
  sVar2 = (uVar3 >> 0xd) * 4;
  sVar4 = *(short *)(&DAT_00004290 + sVar2);
  uVar3 = *(ushort *)(&DAT_00004290 + (short)(sVar2 + 2));
  if ((((0x27 < uVar1) && (DAT_00ffddc6 != DAT_00ffddce)) && (DAT_00ffddc6 != DAT_00ffddca)) &&
     ((DAT_00ffddc8 != DAT_00ffddd0 && (DAT_00ffddc8 != DAT_00ffddcc)))) {
    sVar4 = sVar4 << 1;
    uVar3 = uVar3 << 1;
  }
  if (_DAT_00ffe100 != 0) {
    return;
  }
  if (_DAT_00ffe102 != 0) {
    return;
  }
  if (_DAT_00ffdfe8 != 0) {
    return;
  }
  sVar2 = sVar4 * 0x10;
  if (sVar2 != 0) {
    if (sVar2 < 0) {
      if ((short)(DAT_00ffddc6 + sVar2) < (short)DAT_00ffddca) goto LAB_00003cba;
    }
    else if ((short)DAT_00ffddce < (short)(DAT_00ffddc6 + sVar2)) goto LAB_00003cba;
    DAT_00ffddc6 = DAT_00ffddc6 + sVar2;
    DAT_00ffdd08 = -sVar4 + DAT_00ffdd08;
    DAT_00ffdd0c = -sVar4 + DAT_00ffdd0c;
    uVar1 = DAT_00ffddc6 & 0xff80;
    if (uVar1 != DAT_00ffdf38) {
      uStack_4 = CONCAT22(uVar3,uStack_4._2_2_);
      DAT_00ffdf38 = uVar1;
      FUN_000040e2();
      register0x0000003c = (BADSPACEBASE *)auStack_6;
      uVar3 = uVar1;
    }
  }
LAB_00003cba:
  sVar4 = uVar3 * 0x10;
  if (sVar4 != 0) {
    if (sVar4 < 0) {
      if ((short)(DAT_00ffddc8 + sVar4) < (short)DAT_00ffddcc) {
        return;
      }
    }
    else if ((short)DAT_00ffddd0 < (short)(DAT_00ffddc8 + sVar4)) {
      return;
    }
    DAT_00ffddc8 = DAT_00ffddc8 + sVar4;
    DAT_00ffdd0a = uVar3 + DAT_00ffdd0a;
    DAT_00ffdd0e = uVar3 + DAT_00ffdd0e;
    if ((DAT_00ffddc8 & 0xff80) != DAT_00ffdf36) {
      *(undefined4 *)((int)register0x0000003c + -4) = 0x3d16;
      DAT_00ffdf36 = DAT_00ffddc8 & 0xff80;
      FUN_00004018();
    }
  }
  return;
}


