// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000DBA


undefined8 FUN_00000dba(void)

{
  undefined4 in_D0;
  short sVar1;
  undefined4 in_D1;
  short sVar2;
  short unaff_D4w;
  short unaff_D6w;
  short sVar3;
  
  sVar3 = unaff_D6w + -1;
  if (sVar3 < 0) {
    sVar3 = 0;
  }
  DAT_00fff0dd = '\0';
  do {
    sVar2 = 0;
    while( true ) {
      sVar1 = FUN_00000d3a();
      if ((short)(sVar1 + 1) != 6) break;
      sVar2 = sVar2 + 6;
    }
    if (-1 < (short)((sVar2 + sVar1 + 1) - unaff_D4w)) {
      DAT_00fff0dd = DAT_00fff0dd + '\x01';
    }
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  return CONCAT44(in_D0,in_D1);
}


