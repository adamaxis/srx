// Ghidra headless export
// Entry: 0x000011F6
// Name: FUN_000011f6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short FUN_000011f6(void)

{
  short extraout_D1w;
  ushort unaff_D2w;
  short unaff_D4w;
  short unaff_D7w;
  short sVar1;
  ushort *extraout_A0;
  ushort *extraout_A1;
  ushort *puVar2;
  
  VDP_CTRL = 0x8f80;
  do {
    make_vdp_transfer_command();
    sVar1 = unaff_D7w;
    puVar2 = extraout_A1;
    do {
      *extraout_A0 = unaff_D2w | *puVar2;
      sVar1 = sVar1 + -1;
      puVar2 = puVar2 + 1;
    } while (sVar1 != -1);
    unaff_D4w = unaff_D4w + 2;
  } while (extraout_D1w != 0);
  VDP_CTRL = 0x8f02;
  return unaff_D4w;
}


