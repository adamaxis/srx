// Ghidra headless export
// Entry: 0x0005AD1A
// Name: FUN_0005ad1a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001406 prepare_title_resources
//   0x0000195C sample_controller_input
//   0x0005A96A render_title_intro_text_batch
//   0x0005A9E2 render_title_intro_middle_text_batch
//   0x0005B002 reset_title_intro_scroll_fill
//   0x0005B00A advance_title_intro_scroll
//   0x0005B056 advance_title_intro_scroll_fade
//   0x0005B08E advance_title_intro_triplet
//   0x0005B0BC service_title_intro_animation
//   0x0005B198 reset_title_intro_landing
//   0x0005B29A FUN_0005b29a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ad1a(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  FUN_0005b29a();
  prepare_title_resources();
  DAT_00fff11e = DAT_00ffdf88;
  reset_title_intro_scroll_fill();
  reset_title_intro_landing();
  service_title_intro_animation();
  render_title_intro_middle_text_batch();
  sVar1 = 0x1f;
  puVar2 = &DAT_00ffd8fe;
  puVar3 = &DAT_00ffd87e;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  sample_controller_input();
  sVar1 = 0x528;
  do {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    if (sVar1 == 0x1e0) {
      render_title_intro_text_batch();
    }
    advance_title_intro_triplet();
    advance_title_intro_scroll();
    advance_title_intro_scroll_fade();
    service_title_intro_animation();
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  return;
}


