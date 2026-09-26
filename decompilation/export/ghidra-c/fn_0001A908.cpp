// Ghidra headless export
// Entry: 0x0001A908
// Name: initialize_world_effect_mode
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019B40 select_run_hud_mode


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_world_effect_mode(void)

{
  short sVar1;
  undefined1 *puVar2;
  
  DAT_00ff164a = 1;
  sVar1 = DAT_00ffe0e0;
  if (_DAT_00ffe0de == 0) {
    sVar1 = select_run_hud_mode();
  }
  DAT_00ffe0bc = sVar1;
  if (sVar1 < 0) {
    puVar2 = &DAT_0001aa10;
  }
  else {
    if (sVar1 == 0x45) {
      DAT_00ff164a = 0;
      _DAT_00ffe080 = 1;
      return;
    }
    if (sVar1 == 6) {
      if (DAT_00ffe0b8 == 0) {
        DAT_00ffe0b8 = 1;
        DAT_00ffe0ba = 0x47c0;
      }
    }
    else if ((sVar1 == 7) && (DAT_00ffe0b6 == 0)) {
      DAT_00ffe0b6 = 1;
      DAT_00ffe0be = 0x4800;
    }
    puVar2 = *(undefined1 **)(&DAT_0001a9ce + (short)(sVar1 << 2));
  }
  DAT_00ff1644 = *puVar2;
  DAT_00ff1645 = puVar2[1];
  DAT_00ff1646 = *(undefined4 *)(puVar2 + 2);
  _DAT_00ff1642 = 0;
  DAT_00ff163e = 0x120;
  DAT_00ff1640 = 0xd0;
  return;
}

