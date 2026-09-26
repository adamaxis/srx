// Ghidra headless export
// Entry: 0x0001822C
// Name: run_matrix_interaction_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000DBA count_thresholded_random_rolls
//   0x00000E34 reset_vdp_upload_state
//   0x00000F1C write_transfer_header
//   0x00000F3C write_fixed_transfer_pair
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x00001378 build_title_sprite_records
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x000176EA render_menu_footer
//   0x00017708 render_field_hud
//   0x00017D90 advance_hud_panel
//   0x00017F80 refresh_mode_metadata_wrapper
//   0x00018776 advance_hud_fade_service
//   0x000187AA render_hud_indicator_grids
//   0x0001882E convert_world_routine_0x0001882e
//   0x0001897C write_hud_repeated_grid_variant_a
//   0x000189D8 write_two_hud_grids
//   0x00018AA6 run_cyber_info_screen
//   0x00019238 run_matrix_program_slot_screen
//   0x00019504 write_hud_panel_grid
//   0x000196F2 rotate_hud_history
//   0x00019B40 convert_world_routine_0x00019b40
//   0x00019BF6 pass_through_value
//   0x00026A00 convert_world_routine_0x00026a00
//   0x000530DE service_audio_if_enabled


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_matrix_interaction_loop(void)

{
  char cVar1;
  char cVar2;
  undefined4 uVar3;
  undefined1 uVar4;
  undefined4 uVar5;
  uint uVar6;
  short sVar8;
  uint uVar7;
  ushort uVar9;
  uint unaff_D7;
  int extraout_A1;
  
  uVar4 = DAT_00fff0b7;
  uVar3 = _DAT_00ffdd08;
  DAT_00fff0b1 = 0;
  _DAT_00ffdd08 = 0;
  write_transfer_header();
  write_fixed_transfer_pair();
  VDP_CTRL = 0x8134;
  clear_and_fill_plane_a();
  reset_vdp_upload_state();
  decode_and_upload_source_stream();
  refresh_mode_metadata_wrapper();
  render_hud_sprite_stream();
  render_field_hud();
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  _DAT_00fff0d0 = 0;
  write_hud_panel_grid();
  render_hud_indicator_grids();
  DAT_00fff10a = 0;
  DAT_00fff109 = 0;
  DAT_00fff0d1 = 3;
  write_two_hud_grids();
  VDP_CTRL = 0x8174;
  DAT_00fff0d1 = 0;
  uVar7 = unaff_D7 & 0xffff0000;
LAB_0001831a:
  convert_world_routine_0x0001882e();
  DAT_00fff10c = 0xff;
LAB_00018326:
  while( true ) {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    uVar5 = rotate_hud_history();
    if (((DAT_00ffdd22._0_1_ & 1) == 0) && ((DAT_00ffdd22._0_1_ & 2) == 0)) break;
    uVar5 = write_two_hud_grids();
    DAT_00fff0d1 = (char)uVar5 + DAT_00fff0d1;
    if ((short)((ushort)DAT_00fff0d1 << 8) < 0) {
      DAT_00fff0d1 = 3;
    }
    else if (-1 < (char)(DAT_00fff0d1 - 4)) {
      DAT_00fff0d1 = 0;
    }
    DAT_00ffd986 = (short)CONCAT31((int3)((uint)uVar5 >> 8),DAT_00fff0d1) * 0x28 + 0x94;
    uVar6 = (uint)DAT_00ffd986;
LAB_000183e0:
    DAT_00ffd98c = (short)CONCAT31((int3)(uVar6 >> 8),DAT_00fff0d0) * 0x20 + 0x8c;
    DAT_00ffef14 = DAT_00ffef14 | 2;
    convert_world_routine_0x0001882e();
  }
  if ((DAT_00ffdd22._0_1_ & 4) != 0) {
    uVar6 = 0xffff;
LAB_000183b6:
    DAT_00fff0d0 = (char)uVar6 + DAT_00fff0d0;
    if ((short)((ushort)DAT_00fff0d0 << 8) < 0) {
      DAT_00fff0d0 = 3;
    }
    else if (-1 < (char)(DAT_00fff0d0 - 4)) {
      DAT_00fff0d0 = 0;
    }
    goto LAB_000183e0;
  }
  if ((DAT_00ffdd22._0_1_ & 8) != 0) {
    uVar6 = CONCAT31((int3)((uint)uVar5 >> 8),1);
    goto LAB_000183b6;
  }
  if ((DAT_00ffdd22._0_1_ & 0x40) != 0) {
    DAT_00fff0b7 = 0xff;
LAB_0001842e:
    if (DAT_00fff0d1 == 3) {
      if (DAT_00fff0d0 == 3) {
        if (DAT_00fff0b0 != '\0') {
          write_hud_repeated_grid_variant_a();
          sVar8 = convert_world_routine_0x00019b40();
          if ((sVar8 == 8) && (DAT_00ffdf84 != '\0')) {
            render_encoded_title_text();
          }
          render_encoded_title_text();
          render_encoded_title_text();
          DAT_00fff0dc = '\x01';
          do {
            move_wrapped_menu_cursor();
            while( true ) {
              wait_for_vblank();
              latch_controller_events();
              sample_controller_input();
              rotate_hud_history();
              if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
              if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
                 (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
                uVar7 = 0;
                if (DAT_00fff0dc != '\0') goto LAB_0001831a;
                sVar8 = convert_world_routine_0x00019b40();
                if ((sVar8 == 8) && (DAT_00ffdf84 != '\0')) {
                  pass_through_value();
                  count_thresholded_random_rolls();
                  if (DAT_00fff0dd == '\0') {
                    advance_hud_panel();
                    service_audio_if_enabled();
                    uVar7 = 0;
                    if (*(char *)(DAT_00fffc04 + 0x4c) != '\0') goto LAB_0001831a;
                  }
                  else {
                    count_thresholded_random_rolls();
                    if (DAT_00fff0dd == '\0') {
                      cVar1 = *(char *)(extraout_A1 + 0x9c);
                      cVar2 = cVar1 + -3;
                      *(char *)(extraout_A1 + 0x9c) = cVar2;
                      if (cVar2 == '\0' || cVar1 < '\x03') {
                        *(undefined1 *)(extraout_A1 + 0x9c) = 1;
                      }
                      service_audio_if_enabled();
                    }
                  }
                }
                DAT_00fff0dc = -1;
                advance_hud_fade_service();
                convert_world_routine_0x00026a00();
                goto LAB_00018694;
              }
            }
          } while( true );
        }
      }
      else if (((DAT_00fff0d0 != 2) || (DAT_00fff0b0 != '\0')) &&
              ((**(code **)((int)&DAT_000e2dfe + (int)(short)((ushort)DAT_00fff0d0 << 2)))(),
              DAT_00fff0ad != '\0')) {
        DAT_00fff0ad = '\0';
        uVar5 = 0;
        goto LAB_00018670;
      }
    }
    else {
      run_matrix_program_slot_screen();
    }
    goto LAB_00018326;
  }
  if ((DAT_00ffdd22._0_1_ & 0x10) != 0) {
    DAT_00fff0b7 = 0;
    goto LAB_0001842e;
  }
  if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
    DAT_00fff0b7 = 1;
    goto LAB_0001842e;
  }
  if ((DAT_00ffdd22._0_1_ & 0x80) == 0) goto LAB_00018326;
  uVar5 = CONCAT22((short)(uVar7 >> 0x10),0xffff);
LAB_00018670:
  VDP_CTRL = 0x8134;
  reset_vdp_upload_state(uVar5);
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  DAT_00fff10c = 0;
LAB_00018694:
  _DAT_00ffdd08 = uVar3;
  write_transfer_header();
  write_fixed_transfer_pair();
  render_menu_footer();
  uVar7 = 0;
  DAT_00fff108 = '\0';
  sVar8 = 4;
  do {
    uVar9 = (short)CONCAT31((int3)(uVar7 >> 8),DAT_00fff108) << 1;
    uVar7 = (uint)uVar9;
    if (-1 < *(short *)((int)&DAT_00fff0f2 + (int)(short)uVar9)) {
      DAT_00ffdd48 = *(undefined2 *)((int)&DAT_00fff0f2 + (int)(short)uVar9);
      break;
    }
    DAT_00fff108 = DAT_00fff108 + '\x01';
    sVar8 = sVar8 + -1;
  } while (sVar8 != -1);
  DAT_00fff0b7 = uVar4;
  VDP_CTRL = 0x8174;
  return;
}

