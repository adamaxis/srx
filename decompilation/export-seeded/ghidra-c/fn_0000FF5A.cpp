// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000FF5A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ff5a(void)

{
  undefined1 extraout_D1b;
  undefined1 unaff_D7b;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  FUN_000102d4();
  iVar1 = _DAT_00fffc04;
  if ((-1 < *(char *)(_DAT_00fffc04 + 0x56)) && (*(char *)(_DAT_00fffc04 + 0x57) != '\0')) {
    FUN_000125fa();
    *(undefined1 *)(extraout_A0 + 0x4e) = extraout_D1b;
    iVar1 = extraout_A0;
  }
  *(undefined1 *)(iVar1 + 0x57) = unaff_D7b;
  *(char *)(iVar1 + 0x57) = *(char *)(iVar1 + 0x57) + '\x01';
  while (((FUN_00010016(), (DAT_00ffdd22 & 0x20) == 0 && ((DAT_00ffdd22 & 0x10) == 0)) &&
         ((DAT_00ffdd22 & 0x40) == 0))) {
    FUN_00010722();
    if ((((DAT_00ffdd22 & 0x20) != 0) || ((DAT_00ffdd22 & 0x10) != 0)) ||
       ((DAT_00ffdd22 & 0x40) != 0)) break;
  }
  *(undefined1 *)(_DAT_00fffc04 + 0x56) = 0xff;
  FUN_00003404();
  FUN_00003548();
  FUN_000037ec();
  FUN_000036fc();
  *(char *)(extraout_A0_00 + 0x9d) = (char)_DAT_00fff096;
  return;
}


