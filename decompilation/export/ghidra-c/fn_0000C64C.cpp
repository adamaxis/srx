// Ghidra headless export
// Entry: 0x0000C64C
// Name: FUN_0000c64c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00002A32 initialize_field_mode_profile_a
//   0x00002A44 initialize_field_mode_profile_b
//   0x000038FA run_entity_animation_triplet
//   0x0000391A handle_mode_action_event
//   0x0000C802 FUN_0000c802
//   0x0001DED8 FUN_0001ded8
//   0x0001E68C reset_world_effect_globals
//   0x0001F246 run_mode_exit_services


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000c64c(void)

{
  uint uVar1;
  undefined *puVar2;
  
  DAT_00ffe186 = 0;
  DAT_00ffe182 = 0;
  DAT_00fff10c = 0;
  if ((char)DAT_00fffbec == '\x05') {
    if (((DAT_00fffc1c & 1) == 0) || ((DAT_00fffc1c & 0x80) != 0)) goto LAB_0000c716;
    DAT_00ffe183 = 0;
    puVar2 = &DAT_000f3d5c;
LAB_0000c690:
    FUN_0001ded8(puVar2);
  }
  else {
    if ((char)DAT_00fffbec == '\x01') {
      if ((DAT_00fffc1c & 1) == 0) goto LAB_0000c716;
      puVar2 = &DAT_000f359e;
      goto LAB_0000c690;
    }
    if ((((char)DAT_00fffbec != '\x02') && ((char)DAT_00fffbec != '\x04')) ||
       ((DAT_00fffc1c & 1) == 0)) goto LAB_0000c716;
    DAT_00fffc22 = DAT_00fffc22 & 0xfffffdff;
  }
  reset_world_effect_globals();
  run_mode_exit_services();
LAB_0000c716:
  DAT_00ff024c = 0;
  DAT_00ff034c = 0;
  FUN_0000c802();
  DAT_00ff0110 = DAT_00ff0110 & 0xaf;
  DAT_00ff0110 = DAT_00ff0110 | 1;
  DAT_00ff0111 = DAT_00ff0111 & 0xbf;
  _DAT_00ff013a = 0xb45c;
  if ((char)(DAT_00ff014c + -6) < '\0') {
    DAT_00ff014c = '\x06';
  }
  if ((char)(DAT_00ff019c + -8) < '\0') {
    DAT_00ff019c = '\b';
  }
  uVar1 = DAT_00fffbfe >> 3;
  if (-1 < (int)(uVar1 - 5000)) {
    uVar1 = 5000;
  }
  DAT_00fffbfe = DAT_00fffbfe - uVar1;
  run_mode_exit_services();
  run_entity_animation_triplet();
  handle_mode_action_event();
  if ((DAT_00fffc1e & 1) == 0) {
    initialize_field_mode_profile_b();
    return;
  }
  initialize_field_mode_profile_a();
  return;
}


