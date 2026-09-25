// Ghidra headless export
// Entry: 0x00054662
// Name: FUN_00054662
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x00000DBA count_thresholded_random_rolls
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001232 write_vdp_grid
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x00010952 reset_mode_display_buffers
//   0x000176EA render_menu_footer
//   0x000177CC upload_menu_resource_pair
//   0x00017F80 refresh_mode_metadata_wrapper
//   0x0001822C FUN_0001822c
//   0x000532A8 fill_audio_menu_sentinel
//   0x000549E4 write_menu_repeated_grid_service
//   0x000549F6 select_display_status
//   0x00054A28 FUN_00054a28
//   0x00054A8E FUN_00054a8e
//   0x00055F44 FUN_00055f44
//   0x000566F6 refresh_mode_resource_state


/* WARNING: Removing unreachable block (ram,0x000548c4) */

undefined4 FUN_00054662(void)

{
  byte *pbVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  short sVar5;
  short sVar6;
  
  sVar6 = DAT_00fff102;
  DAT_00fff0b7 = 0xff;
  DAT_00fff0d4 = 0;
  *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) =
       *(byte *)((int)&DAT_00fffb30 + (int)DAT_00fff102) & 0xf;
  pbVar1 = (byte *)((int)&DAT_00fffb30 + (int)sVar6);
  *pbVar1 = *pbVar1 | 0x40;
  sVar5 = DAT_00fff118 * 0x14;
  pbVar1 = (byte *)((int)&DAT_00fffb30 + (int)sVar6);
  *pbVar1 = *(byte *)((int)&DAT_00ff1104 + sVar5 + 1) | *pbVar1;
  if (*(char *)((int)&DAT_00ff1100 + (int)sVar5) == '\x01') {
    fill_audio_menu_sentinel();
  }
  DAT_00fff0a6 = 0xff;
  DAT_00fff0a8 = '\0';
  VDP_CTRL = 0x8b00;
LAB_000546cc:
  do {
    VDP_CTRL = 0x8134;
    reset_mode_display_buffers();
    refresh_mode_resource_state();
    write_vdp_word_block_with_source_wrap();
    write_vdp_word_block_with_source_wrap();
    sVar6 = 5;
    do {
      write_vdp_grid();
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
    write_vdp_grid();
    write_vdp_grid();
    write_vdp_grid();
    write_vdp_grid();
    select_display_status();
    refresh_mode_metadata_wrapper();
    VDP_CTRL = 0x8174;
    FUN_00054a8e();
    if (DAT_00fff0b1 < '\0') {
      return 0;
    }
    render_encoded_title_text();
    render_encoded_title_text();
    FUN_00054a28();
    DAT_00fff0dc = 0;
LAB_0005488c:
    move_wrapped_menu_cursor();
    do {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) goto LAB_0005488c;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        FUN_00055f44();
        sVar6 = *(byte *)(DAT_00fffc04 + 0x88) - 1;
        if (sVar6 == 0) {
          sVar6 = 1;
        }
        cVar2 = *(char *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) + -1;
        bVar3 = (char)sVar6 - cVar2;
        if ((bVar3 == 0 || SBORROW1((char)sVar6,cVar2) != (short)((ushort)bVar3 << 8) < 0) ||
           (count_thresholded_random_rolls(), DAT_00fff0dd < '\x01')) {
          write_menu_repeated_grid_service();
          render_encoded_title_text();
          wait_for_controller_event();
          DAT_00fff0a8 = DAT_00fff0a8 + '\x01';
        }
        else {
          iVar4 = (uint)(ushort)(short)*(char *)((int)&DAT_00ff1100 +
                                                (int)(short)(DAT_00fff118 * 0x14)) * 10;
          (**(code **)(*(int *)((int)&PTR_PTR_001664de + (int)(short)iVar4) +
                      (int)(short)((short)CONCAT31((int3)((uint)iVar4 >> 8),DAT_00fff0dc) << 2)))();
          DAT_00fff0a8 = DAT_00fff0a8 + '\x01';
        }
        goto LAB_000546cc;
      }
    } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
    VDP_CTRL = 0x8134;
    DAT_00fff0b7 = 0;
    render_menu_footer();
    upload_menu_resource_pair();
    VDP_CTRL = 0x8174;
    FUN_0001822c();
    sVar6 = 4;
    do {
      (&DAT_00fff0ed)[sVar6] = 6;
      (&DAT_00fff0e8)[sVar6] = 1;
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
  } while( true );
}


