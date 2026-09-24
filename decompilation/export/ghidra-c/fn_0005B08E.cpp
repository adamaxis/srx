// Ghidra headless export
// Entry: 0x0005B08E
// Name: advance_title_intro_triplet
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort advance_title_intro_triplet(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar1 = DAT_00ffd89e._2_2_;
  uVar2 = DAT_00ffdf88 & 7;
  if ((DAT_00ffdf88 & 7) == 0) {
    DAT_00ffd89e._2_2_ = DAT_00ffd8a2._0_2_;
    DAT_00ffd8a2._0_2_ = DAT_00ffd8a2._2_2_;
    DAT_00ffd8a2._2_2_ = uVar1;
    DAT_00ffef14 = DAT_00ffef14 | 1;
    uVar2 = uVar1;
  }
  return uVar2;
}


