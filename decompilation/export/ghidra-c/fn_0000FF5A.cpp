// Ghidra headless export
// Entry: 0x0000FF5A
// Name: FUN_0000ff5a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 FUN_00003404
//   0x00003548 FUN_00003548
//   0x000036FC FUN_000036fc
//   0x000037EC FUN_000037ec
//   0x00010016 FUN_00010016
//   0x000102D4 FUN_000102d4
//   0x00010722 FUN_00010722
//   0x000125FA FUN_000125fa


void FUN_0000ff5a(void)

{
  undefined1 extraout_D1b;
  undefined1 unaff_D7b;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  FUN_000102d4();
  iVar1 = DAT_00fffc04;
  if ((-1 < *(char *)(DAT_00fffc04 + 0x56)) && (*(char *)(DAT_00fffc04 + 0x57) != '\0')) {
    FUN_000125fa();
    *(undefined1 *)(extraout_A0 + 0x4e) = extraout_D1b;
    iVar1 = extraout_A0;
  }
  *(undefined1 *)(iVar1 + 0x57) = unaff_D7b;
  *(char *)(iVar1 + 0x57) = *(char *)(iVar1 + 0x57) + '\x01';
  while (((FUN_00010016(), (DAT_00ffdd22._0_1_ & 0x20) == 0 && ((DAT_00ffdd22._0_1_ & 0x10) == 0))
         && ((DAT_00ffdd22._0_1_ & 0x40) == 0))) {
    FUN_00010722();
    if ((((DAT_00ffdd22._0_1_ & 0x20) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
       ((DAT_00ffdd22._0_1_ & 0x40) != 0)) break;
  }
  *(undefined1 *)(DAT_00fffc04 + 0x56) = 0xff;
  FUN_00003404();
  FUN_00003548();
  FUN_000037ec();
  FUN_000036fc();
  *(char *)(extraout_A0_00 + 0x9d) = (char)DAT_00fff096;
  return;
}


