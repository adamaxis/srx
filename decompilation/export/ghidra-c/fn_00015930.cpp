// Ghidra headless export
// Entry: 0x00015930
// Name: apply_entity_secondary_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000049A0 noop_effect_callback
//   0x0001582A allocate_first_free_effect_slot_local
//   0x000530DE service_audio_if_enabled


void apply_entity_secondary_effect(void)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short extraout_D1w_00;
  int extraout_A1;
  undefined1 in_CF;
  
  uVar1 = allocate_first_free_effect_slot_local();
  if (!(bool)in_CF) {
    *(undefined1 *)(extraout_A1 + 0xb2) = 2;
    *(undefined2 *)(extraout_A1 + 0x32) = uVar1;
    *(undefined2 *)(extraout_A1 + 0x2a) = extraout_D1w;
    *(undefined1 *)(extraout_A1 + 2) = 0;
    *(undefined1 *)(extraout_A1 + 3) = 0;
    sVar2 = 2;
    if (u_44178_0000494a[0] != L'\0') {
      service_audio_if_enabled();
      sVar2 = extraout_D1w_00;
    }
    (**(code **)((int)&PTR_noop_effect_callback_00004964 + (int)(short)(sVar2 << 2)))();
  }
  return;
}


