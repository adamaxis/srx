// Ghidra headless export
// Entry: 0x00019238
// Name: FUN_00019238
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x0000195C FUN_0000195c
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 FUN_0000c376
//   0x0001882E FUN_0001882e
//   0x00018998 FUN_00018998
//   0x00018A92 FUN_00018a92
//   0x000194AC FUN_000194ac
//   0x00019504 FUN_00019504
//   0x0001952C FUN_0001952c
//   0x0001957E FUN_0001957e
//   0x000195D8 FUN_000195d8
//   0x0001962A FUN_0001962a
//   0x000196F2 FUN_000196f2
//   0x00019736 FUN_00019736


void FUN_00019238(void)

{
  short extraout_D1w;
  short unaff_D6w;
  short unaff_D7w;
  short sVar1;
  undefined1 *puVar2;
  
  FUN_00018a92();
  if ((&DAT_00fffbcc)[unaff_D7w] != '\0') {
    DAT_00ffdd50 = 0;
    DAT_00fff0dc = '\0';
    FUN_00018998();
    FUN_00018a92();
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    do {
      if ((char)unaff_D7w == puVar2[0x1a]) {
        DAT_00fff0d4 = '\x04' - (char)sVar1;
        FUN_0000c0da();
        FUN_0000c0da();
        FUN_0000c0da();
        DAT_00ffdd50 = -1;
        DAT_00fff0dc = '\x01';
        FUN_000194ac();
        FUN_0000c0da();
        do {
          FUN_0000c376();
          while( true ) {
            FUN_00000bd2();
            FUN_00000bf2();
            FUN_0000195c();
            FUN_000196f2();
            if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
            if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
               (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
              if (DAT_00fff0dc != '\0') goto LAB_00019428;
              if (DAT_00ffdd50 == 0) goto LAB_00019410;
              FUN_0001962a();
              goto LAB_00019428;
            }
            if ((DAT_00ffdd22._0_1_ & 0x80) != 0) goto LAB_00019428;
          }
        } while( true );
      }
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    while (-1 < (char)puVar2[0x1a]) {
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
      if (sVar1 == -1) {
        FUN_0000c0da();
        FUN_0000c0da();
        return;
      }
    }
    DAT_00fff0d4 = '\x04' - (char)sVar1;
    FUN_0001957e();
    FUN_00018a92();
    FUN_0001952c();
    if ((short)(unaff_D6w - extraout_D1w) < 0) {
      FUN_0000c0da();
      FUN_0000c0da();
      return;
    }
LAB_00019410:
    FUN_00019736();
    FUN_000195d8();
LAB_00019428:
    FUN_00019504();
    FUN_0001882e();
  }
  return;
}


