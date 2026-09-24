// Ghidra headless export
// Entry: 0x00010D30
// Name: FUN_00010d30
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00000E34 FUN_00000e34
//   0x0000195C FUN_0000195c
//   0x0000391A FUN_0000391a
//   0x0000C0C2 FUN_0000c0c2
//   0x00010F50 FUN_00010f50
//   0x000110BC FUN_000110bc
//   0x0001111A FUN_0001111a
//   0x000111D0 FUN_000111d0
//   0x000111E8 FUN_000111e8
//   0x00011454 FUN_00011454
//   0x000114A0 FUN_000114a0
//   0x00011506 FUN_00011506
//   0x00011592 FUN_00011592
//   0x00011624 FUN_00011624
//   0x0001170C FUN_0001170c
//   0x00011740 FUN_00011740
//   0x00011928 FUN_00011928
//   0x000530DE FUN_000530de
//   0x0005670E FUN_0005670e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00010d30(void)

{
  undefined2 uVar1;
  
  uVar1 = _DAT_00fff0d0;
code_r0x00010d30:
  _DAT_00fff0d0 = uVar1;
  if (((*(byte *)(DAT_00fffc04 + 0x10) & 0x40) != 0) ||
     ((*(byte *)(DAT_00fffc04 + 0x10) & 0x10) != 0)) {
    FUN_0000391a();
  }
  DAT_00fff09d = 0xff;
  DAT_00fff0b1 = '\0';
  _DAT_00fff112 = 0;
  VDP_CTRL = 0x8134;
  DAT_00fff0e7 = 0;
  DAT_00fff100 = 0;
  FUN_00010f50();
  FUN_0005670e();
  FUN_0000c0c2();
  VDP_CTRL = 0x8174;
  uVar1 = _DAT_00fff0d0;
LAB_00010d90:
  _DAT_00fff0d0 = uVar1;
  DAT_00fff0e0 = 0;
  FUN_000110bc();
  FUN_0001111a();
  FUN_00011928();
  FUN_00011624();
  FUN_000111d0();
  FUN_00011454();
  _DAT_00fff112 = CONCAT11(0xff,DAT_00fff112_1);
LAB_00010e2e:
  do {
    FUN_000530de();
    FUN_0001170c();
    FUN_000111e8();
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if ((DAT_00ffdd22._0_1_ & 4) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 1) != 0) {
LAB_00010e20:
        FUN_00011506();
        if (DAT_00fff0b1 != '\0') goto LAB_00010f22;
        goto LAB_00010e2e;
      }
      if ((DAT_00ffdd22._0_1_ & 8) != 0) break;
      if ((DAT_00ffdd22._0_1_ & 2) != 0) goto LAB_00010e20;
      if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
        if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
          FUN_0000391a();
          FUN_00000e34();
          _DAT_00fff0d0 = 0x600;
          FUN_000114a0();
          DAT_00fff115 = 0xff;
          FUN_00011506();
          uVar1 = _DAT_00fff0d0;
          goto LAB_00010d90;
        }
        if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
LAB_00010f22:
          if (((*(byte *)(DAT_00fffc04 + 0x10) & 0x40) != 0) ||
             ((*(byte *)(DAT_00fffc04 + 0x10) & 0x10) != 0)) {
            FUN_0000391a();
          }
          DAT_00fff09d = 0;
          DAT_00fff0b1 = 0;
          return;
        }
      }
      else {
        FUN_000530de();
        uVar1 = _DAT_00fff0d0;
        FUN_00011740();
        DAT_00fff0d0 = SUB21(uVar1,1);
        _DAT_00fff0d0 = uVar1;
        if ('\x01' < DAT_00fff0d0) {
          if (DAT_00fff0d0 == '\x03') goto code_r0x00010d30;
          if (DAT_00fff0d0 == '\x06') {
            if (DAT_00fff0cd != '\0') {
              DAT_00fff0cd = '\0';
              goto LAB_00010f22;
            }
          }
          else if (('\x01' < *(char *)(DAT_00fffc04 + 0x9b)) || (DAT_00fff0d0 != '\x05'))
          goto LAB_00010d90;
          goto code_r0x00010d30;
        }
        FUN_00011592();
      }
    }
    FUN_000114a0();
  } while( true );
}


