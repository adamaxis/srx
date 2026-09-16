// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013228


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_00013228(void)

{
  byte bVar1;
  undefined4 in_D0;
  char cVar2;
  undefined4 in_D1;
  ushort uVar3;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int iVar4;
  
  bVar1 = DAT_00fff0d0;
  cVar2 = *(char *)(_DAT_00fffc04 + 0x66 + (int)(short)(ushort)DAT_00fff0d0);
  FUN_00012a54();
  FUN_00001232();
  if ((char)(cVar2 + -0xd) < '\0') {
    uVar3 = (ushort)bVar1;
    iVar4 = extraout_A0;
    if ((*(byte *)(extraout_A0 + 0x6e + (int)(short)uVar3) & 2) != 0) {
      cVar2 = FUN_00001232();
      iVar4 = extraout_A0_00;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 4) != 0) {
      cVar2 = FUN_00001232();
      iVar4 = extraout_A0_01;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 1) != 0) {
      cVar2 = FUN_00001232();
      iVar4 = extraout_A0_02;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 8) != 0) {
      cVar2 = FUN_00001232();
      iVar4 = extraout_A0_03;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 0x10) != 0) {
      cVar2 = FUN_00001232();
      iVar4 = extraout_A0_04;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 0x20) != 0) {
      cVar2 = FUN_00001232();
    }
  }
  if (-1 < (char)(cVar2 + -0xd)) {
    if (((-1 < (char)(cVar2 + -0x14)) && (cVar2 != '\x14')) &&
       (((char)(cVar2 + -0x18) < '\0' || (-1 < (char)(cVar2 + -0x29))))) {
      return CONCAT44(in_D0,in_D1);
    }
    FUN_00012148();
  }
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


