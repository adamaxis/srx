// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000130C0


undefined8 FUN_000130c0(void)

{
  undefined4 in_D0;
  char cVar1;
  undefined4 in_D1;
  char *in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int iVar2;
  
  cVar1 = *in_A0;
  FUN_00012a54();
  FUN_00001232();
  if ((char)(cVar1 + -0xc) < '\0') {
    iVar2 = extraout_A0;
    if ((*(byte *)(extraout_A0 + 1) & 2) != 0) {
      cVar1 = FUN_00001232();
      iVar2 = extraout_A0_00;
    }
    if ((*(byte *)(iVar2 + 1) & 4) != 0) {
      cVar1 = FUN_00001232();
      iVar2 = extraout_A0_01;
    }
    if ((*(byte *)(iVar2 + 1) & 1) != 0) {
      cVar1 = FUN_00001232();
      iVar2 = extraout_A0_02;
    }
    if ((*(byte *)(iVar2 + 1) & 8) != 0) {
      cVar1 = FUN_00001232();
      iVar2 = extraout_A0_03;
    }
    if ((*(byte *)(iVar2 + 1) & 0x10) != 0) {
      cVar1 = FUN_00001232();
      iVar2 = extraout_A0_04;
    }
    if ((*(byte *)(iVar2 + 1) & 0x20) != 0) {
      cVar1 = FUN_00001232();
    }
  }
  if ((((-1 < (char)(cVar1 + -0xc)) && (-1 < (char)(cVar1 + -0x13))) && (cVar1 != '\x13')) &&
     (((char)(cVar1 + -0x17) < '\0' || (-1 < (char)(cVar1 + -0x28))))) {
    return CONCAT44(in_D0,in_D1);
  }
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


